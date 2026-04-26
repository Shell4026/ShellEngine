#include "ComputeShaderParser.h"

#include <fmt/core.h>

#include <cctype>

namespace sh::render
{
	auto ComputeShaderParser::PeekToken() const -> const ComputeShaderLexer::Token&
	{
		return (*tokens)[pos];
	}
	auto ComputeShaderParser::CheckToken(ComputeShaderLexer::TokenType token) const -> bool
	{
		return PeekToken().type == token;
	}
	auto ComputeShaderParser::NextToken() -> const ComputeShaderLexer::Token&
	{
		if (pos < tokens->size())
			return (*tokens)[pos++];
		return (*tokens)[pos];
	}
	auto ComputeShaderParser::PreviousToken() -> const ComputeShaderLexer::Token&
	{
		return (*tokens)[pos - 1];
	}
	void ComputeShaderParser::ConsumeToken(ComputeShaderLexer::TokenType token)
	{
		if (CheckToken(token))
			NextToken();
		else
			throw ComputeShaderParserException{ GetTokenErrorString(fmt::format("Unexpected token '{}'", PeekToken().text)).c_str() };
	}
	void ComputeShaderParser::ConsumeToken(const std::initializer_list<ComputeShaderLexer::TokenType>& tokens)
	{
		for (ComputeShaderLexer::TokenType token : tokens)
		{
			if (CheckToken(token))
			{
				NextToken();
				return;
			}
		}
		throw ComputeShaderParserException{ GetTokenErrorString(fmt::format("Unexpected token '{}'", PeekToken().text)).c_str() };
	}

	auto ComputeShaderParser::GetCurrentTokenPosString() const -> std::string
	{
		const ComputeShaderLexer::Token& tk = PeekToken();
		return fmt::format("(line: {}, col: {})", tk.line, tk.column);
	}
	auto ComputeShaderParser::GetTokenErrorString(const std::string& msg) const -> std::string
	{
		const ComputeShaderLexer::Token& tk = PeekToken();
		return fmt::format("{} (line: {}, col: {})", msg, tk.line, tk.column);
	}

	auto ComputeShaderParser::ParseComputeShader() -> ShaderAST::ComputeShaderNode
	{
		ShaderAST::ComputeShaderNode node;

		while (CheckToken(ComputeShaderLexer::TokenType::Preprocessor))
		{
			NextToken();
			ParsePreprocessor(node);
		}

		while (!CheckToken(ComputeShaderLexer::TokenType::EndOfFile))
		{
			if (CheckToken(ComputeShaderLexer::TokenType::Numthreads))
				ParseNumthreads(node);
			else if (CheckToken(ComputeShaderLexer::TokenType::RBuffer))
				ParseBuffer(node, ShaderAST::BufferAccess::Read);
			else if (CheckToken(ComputeShaderLexer::TokenType::WBuffer))
				ParseBuffer(node, ShaderAST::BufferAccess::Write);
			else if (CheckToken(ComputeShaderLexer::TokenType::RWBuffer))
				ParseBuffer(node, ShaderAST::BufferAccess::ReadWrite);
			else if (CheckToken(ComputeShaderLexer::TokenType::Const))
			{
				NextToken();
				ParseDeclaration(node, "const");
			}
			else if (CheckToken(ComputeShaderLexer::TokenType::Identifier))
				ParseDeclaration(node);
			else
				NextToken();
		}

		return node;
	}

	void ComputeShaderParser::ParsePreprocessor(ShaderAST::ComputeShaderNode& node)
	{
		ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
		if (PreviousToken().text == "version")
		{
			ShaderAST::VersionNode versionNode{};
			ConsumeToken(ComputeShaderLexer::TokenType::Number);
			versionNode.versionNumber = std::stoi(PreviousToken().text);
			ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
			versionNode.profile = PreviousToken().text;
			node.version = versionNode;
			return;
		}
		throw ComputeShaderParserException{
			fmt::format("Unknown preprocessor identifier {} {}", PreviousToken().text, GetCurrentTokenPosString()).c_str()
		};
	}

	void ComputeShaderParser::ParseNumthreads(ShaderAST::ComputeShaderNode& node)
	{
		// Numthreads(x, y, z);
		ConsumeToken(ComputeShaderLexer::TokenType::Numthreads);
		ConsumeToken(ComputeShaderLexer::TokenType::LBracket);

		ConsumeToken(ComputeShaderLexer::TokenType::Number);
		node.numthreadsX = static_cast<uint32_t>(std::stoi(PreviousToken().text));
		ConsumeToken(ComputeShaderLexer::TokenType::Comma);
		ConsumeToken(ComputeShaderLexer::TokenType::Number);
		node.numthreadsY = static_cast<uint32_t>(std::stoi(PreviousToken().text));
		ConsumeToken(ComputeShaderLexer::TokenType::Comma);
		ConsumeToken(ComputeShaderLexer::TokenType::Number);
		node.numthreadsZ = static_cast<uint32_t>(std::stoi(PreviousToken().text));

		ConsumeToken(ComputeShaderLexer::TokenType::RBracket);
		ConsumeToken(ComputeShaderLexer::TokenType::Semicolon);
	}

	void ComputeShaderParser::ParseBuffer(ShaderAST::ComputeShaderNode& node, ShaderAST::BufferAccess access)
	{
		// (R|W|RW)Buffer<TYPE> name;
		NextToken(); // RBuffer / WBuffer / RWBuffer

		// '<'
		if (!CheckToken(ComputeShaderLexer::TokenType::Operator) || PeekToken().text != "<")
			throw ComputeShaderParserException{ GetTokenErrorString("Expected '<' for buffer template").c_str() };
		NextToken();

		// 원소 타입
		ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
		ShaderAST::VariableType elementType = IdentifierToVariableType(PreviousToken());

		// '>'
		if (!CheckToken(ComputeShaderLexer::TokenType::Operator) || PeekToken().text != ">")
			throw ComputeShaderParserException{ GetTokenErrorString("Expected '>' for buffer template").c_str() };
		NextToken();

		// 버퍼 이름
		ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
		std::string bufferName = PreviousToken().text;

		ConsumeToken(ComputeShaderLexer::TokenType::Semicolon);

		ShaderAST::BufferNode buffer{};
		buffer.bufferType = ShaderAST::BufferType::Storage;
		buffer.access = access;
		buffer.bAnonymousInstance = true;
		buffer.set = 0;
		buffer.binding = lastBinding++;
		buffer.name = bufferName;
		buffer.vars.push_back(ShaderAST::VariableNode::MakeDynamicArray(elementType, bufferName));

		node.buffers.push_back(std::move(buffer));
	}

	void ComputeShaderParser::ParseDeclaration(ShaderAST::ComputeShaderNode& node, const std::string& qualifier)
	{
		// <ident> <ident> <rest>
		// <rest> ::= ';' | '=' <expr> ';' | '(' <parameters> ')' <function body>
		ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
		const std::string type = PreviousToken().text;
		ConsumeToken(ComputeShaderLexer::TokenType::Identifier);
		const std::string name = PreviousToken().text;
		bool array = false;
		std::size_t arraySize = 0;
		if (CheckToken(ComputeShaderLexer::TokenType::LSquareBracket))
		{
			NextToken();
			ConsumeToken(ComputeShaderLexer::TokenType::Number);
			arraySize = std::stoi(PreviousToken().text);
			ConsumeToken(ComputeShaderLexer::TokenType::RSquareBracket);
			array = true;
		}
		ConsumeToken({ ComputeShaderLexer::TokenType::Semicolon,
					   ComputeShaderLexer::TokenType::Operator,
					   ComputeShaderLexer::TokenType::LBracket });
		auto& prevToken = PreviousToken();
		if (prevToken.type == ComputeShaderLexer::TokenType::Semicolon) // 변수 선언
		{
			if (!array)
				node.declaration.push_back(fmt::format("{} {} {};", qualifier, type, name));
			else
				node.declaration.push_back(fmt::format("{} {} {}[{}];", qualifier, type, name, arraySize));
		}
		else if (prevToken.type == ComputeShaderLexer::TokenType::Operator) // 변수 선언+초기화
		{
			if (prevToken.text != "=")
				throw ComputeShaderParserException{ GetTokenErrorString("Expected '='").c_str() };
			std::string expr;
			while (!CheckToken(ComputeShaderLexer::TokenType::Semicolon))
			{
				if (PeekToken().type == ComputeShaderLexer::TokenType::Literal)
					expr.pop_back();
				expr += PeekToken().text;
				expr += " ";
				NextToken();
			}
			NextToken(); // ;
			if (!array)
				node.declaration.push_back(fmt::format("{} {} {} = {};", qualifier, type, name, std::move(expr)));
			else
				node.declaration.push_back(fmt::format("{} {} {}[{}] = {};", qualifier, type, name, arraySize, std::move(expr)));
		}
		else if (prevToken.type == ComputeShaderLexer::TokenType::LBracket) // 함수 정의
		{
			std::string parameters;
			while (!CheckToken(ComputeShaderLexer::TokenType::RBracket))
			{
				if (PeekToken().type == ComputeShaderLexer::TokenType::Literal)
					parameters.pop_back();
				parameters += PeekToken().text;
				parameters += " ";
				NextToken();
			}
			ConsumeToken(ComputeShaderLexer::TokenType::RBracket);
			ConsumeToken(ComputeShaderLexer::TokenType::LBrace);
			std::string body = ParseFunctionBody();
			ConsumeToken(ComputeShaderLexer::TokenType::RBrace);
			node.functions.push_back(fmt::format("{} {} {}({}) {{ {} }}", qualifier, type, name, std::move(parameters), std::move(body)));
		}
	}

	auto ComputeShaderParser::ParseFunctionBody() -> std::string
	{
		std::string code;
		int nested = 1;
		while (PeekToken().type != ComputeShaderLexer::TokenType::EndOfFile)
		{
			auto& token = PeekToken();
			if (token.type == ComputeShaderLexer::TokenType::RBrace)
			{
				if (--nested == 0)
					break;
			}
			else if (token.type == ComputeShaderLexer::TokenType::LBrace)
				++nested;
			else if (token.type == ComputeShaderLexer::TokenType::Literal)
			{
				if (!code.empty())
					code.pop_back();
			}
			code += token.text + " ";
			NextToken();
		}
		return code;
	}

	auto ComputeShaderParser::IdentifierToVariableType(const ComputeShaderLexer::Token& token) -> ShaderAST::VariableType
	{
		const std::string& type = token.text;
		if (type == "vec4") return ShaderAST::VariableType::Vec4;
		if (type == "vec3") return ShaderAST::VariableType::Vec3;
		if (type == "vec2") return ShaderAST::VariableType::Vec2;
		if (type == "mat4") return ShaderAST::VariableType::Mat4;
		if (type == "mat3") return ShaderAST::VariableType::Mat3;
		if (type == "mat2") return ShaderAST::VariableType::Mat2;
		if (type == "ivec4") return ShaderAST::VariableType::IVec4;
		if (type == "int") return ShaderAST::VariableType::Int;
		if (type == "float") return ShaderAST::VariableType::Float;
		if (type == "bool") return ShaderAST::VariableType::Boolean;
		else return ShaderAST::VariableType::Struct;
	}
	auto ComputeShaderParser::VariableTypeToString(ShaderAST::VariableType type) -> std::string
	{
		switch (type)
		{
		case ShaderAST::VariableType::Mat4: return "mat4";
		case ShaderAST::VariableType::Mat3: return "mat3";
		case ShaderAST::VariableType::Mat2: return "mat2";
		case ShaderAST::VariableType::Vec4: return "vec4";
		case ShaderAST::VariableType::Vec3: return "vec3";
		case ShaderAST::VariableType::Vec2: return "vec2";
		case ShaderAST::VariableType::IVec4: return "ivec4";
		case ShaderAST::VariableType::Float: return "float";
		case ShaderAST::VariableType::Int: return "int";
		case ShaderAST::VariableType::Sampler: return "sampler2D";
		case ShaderAST::VariableType::Boolean: return "bool";
		default: return "unknown";
		}
	}

	void ComputeShaderParser::GenerateCode(ShaderAST::ComputeShaderNode& node)
	{
		std::string code = fmt::format("#version {} {}\n", node.version.versionNumber, node.version.profile);
		code += fmt::format("layout(local_size_x = {}, local_size_y = {}, local_size_z = {}) in;\n",
			node.numthreadsX, node.numthreadsY, node.numthreadsZ);

		for (auto& buf : node.buffers)
		{
			std::string members;
			for (ShaderAST::VariableNode& v : buf.vars)
			{
				if (v.bDynamicArray)
					members += fmt::format("    {} {}[];\n", VariableTypeToString(v.type), v.name);
				else if (v.arraySize == 1)
					members += fmt::format("    {} {};\n", VariableTypeToString(v.type), v.name);
				else
					members += fmt::format("    {} {}[{}];\n", VariableTypeToString(v.type), v.name, v.arraySize);
			}

			const char* accessQualifier = "";
			switch (buf.access)
			{
			case ShaderAST::BufferAccess::Read: accessQualifier = "readonly "; break;
			case ShaderAST::BufferAccess::Write: accessQualifier = "writeonly "; break;
			case ShaderAST::BufferAccess::ReadWrite: accessQualifier = ""; break;
			}

			if (buf.bufferType == ShaderAST::BufferType::Storage)
			{
				if (buf.bAnonymousInstance)
				{
					code += fmt::format("layout(std430, binding = {}) {}buffer BUFFER_{} {{\n{}}};\n",
						buf.binding, accessQualifier, buf.name, members);
				}
				else
				{
					code += fmt::format("layout(std430, set = {}, binding = {}) {}buffer UNIFORM_{} {{\n{}}} {};\n",
						buf.set, buf.binding, accessQualifier, buf.name, members, buf.name);
				}
			}
			else if (buf.bufferType == ShaderAST::BufferType::Uniform)
			{
				code += fmt::format("layout(set = {}, binding = {}) uniform UNIFORM_{} {{\n{}}} {};\n",
					buf.set, buf.binding, buf.name, members, buf.name);
			}
		}
		for (auto& decl : node.declaration)
			code += decl + '\n';
		for (auto& fn : node.functions)
			code += fn + '\n';

		node.code = std::move(code);
	}

	SH_RENDER_API auto ComputeShaderParser::Parse(const std::vector<ComputeShaderLexer::Token>& tokens) -> ShaderAST::ComputeShaderNode
	{
		this->tokens = &tokens;
		pos = 0;
		lastBinding = 0;

		ShaderAST::ComputeShaderNode node = ParseComputeShader();
		GenerateCode(node);
		return node;
	}
}//namespace
