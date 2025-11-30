#include "ShaderParser.h"
#include "StencilState.h"
#include "UniformStructLayout.h"
#include "Mesh.h"

#include <fmt/core.h>

#include <cctype>
#include <array>
#include <algorithm>

namespace sh::render
{
	auto ShaderParser::PeekToken() const -> const ShaderLexer::Token&
	{
		return (*tokens)[pos];
	}
	auto ShaderParser::CheckToken(ShaderLexer::TokenType token) const -> bool
	{
		return (PeekToken().type == token);
	}
	auto ShaderParser::NextToken() -> const ShaderLexer::Token&
	{
		if (pos < tokens->size())
			return (*tokens)[pos++];
		return (*tokens)[pos];
	}
	auto ShaderParser::PreviousToken() -> const ShaderLexer::Token&
	{
		return (*tokens)[pos - 1];
	}
	void ShaderParser::ConsumeToken(ShaderLexer::TokenType token)
	{
		if (CheckToken(token))
			NextToken();
		else
		{
			const ShaderLexer::Token& tk = PeekToken();
			throw ShaderParserException{ GetNotFoundTokenString({ token }).c_str() };
		}
	}
	void ShaderParser::ConsumeToken(const std::initializer_list<ShaderLexer::TokenType>& tokens)
	{
		for (auto token : tokens)
		{
			if (CheckToken(token))
			{
				NextToken();
				return;
			}
		}
		const ShaderLexer::Token& tk = PeekToken();
		throw ShaderParserException{ GetNotFoundTokenString(tokens).c_str() };
	}

	auto ShaderParser::IdentifierToVaraibleType(const ShaderLexer::Token& token) const -> ShaderAST::VariableType
	{
		auto& type = token.text;
		if (type == "vec4")
			return ShaderAST::VariableType::Vec4;
		if (type == "vec3")
			return ShaderAST::VariableType::Vec3;
		if (type == "vec2")
			return ShaderAST::VariableType::Vec2;
		if (type == "mat4")
			return ShaderAST::VariableType::Mat4;
		if (type == "mat3")
			return ShaderAST::VariableType::Mat3;
		if (type == "mat2")
			return ShaderAST::VariableType::Mat2;
		if (type == "int")
			return ShaderAST::VariableType::Int;
		if (type == "float")
			return ShaderAST::VariableType::Float;
		if (type == "bool")
			return ShaderAST::VariableType::Boolean;
		throw ShaderParserException{ GetTokenErrorString("Not found valid variable token").c_str() };
	}
	auto ShaderParser::VariableTypeToString(ShaderAST::VariableType type) const -> std::string
	{
		switch (type)
		{
		case ShaderAST::VariableType::Mat4:
			return "mat4";
		case ShaderAST::VariableType::Mat3:
			return "mat3";
		case ShaderAST::VariableType::Mat2:
			return "mat2";
		case ShaderAST::VariableType::Vec4:
			return "vec4";
		case ShaderAST::VariableType::Vec3:
			return "vec3";
		case ShaderAST::VariableType::Vec2:
			return "vec2";
		case ShaderAST::VariableType::Float:
			return "float";
		case ShaderAST::VariableType::Int:
			return "int";
		case ShaderAST::VariableType::Sampler:
			return "sampler2D";
		case ShaderAST::VariableType::Boolean:
			return "bool";
		default:
			return "unknown";
		}
	}
	auto ShaderParser::GetCurrentTokenPosString() const -> std::string
	{
		const ShaderLexer::Token& tk = PeekToken();
		return fmt::format("(line: {}, col: {})", tk.line, tk.column);
	}
	auto ShaderParser::GetNotFoundTokenString(const std::initializer_list<ShaderLexer::TokenType>& tokens) -> std::string
	{
		std::string msg = "Not found token (";
		for (auto token : tokens)
		{
			if (token == ShaderLexer::TokenType::LBracket)
				msg += "'{'";
			else if (token == ShaderLexer::TokenType::RBracket)
				msg += "'}'";
			else if (token == ShaderLexer::TokenType::LBrace)
				msg += "'('";
			else if (token == ShaderLexer::TokenType::RBrace)
				msg += "')'";
			else if (token == ShaderLexer::TokenType::Pass)
				msg += "Pass";
			else if (token == ShaderLexer::TokenType::Stage)
				msg += "'Stage'";
			else if (token == ShaderLexer::TokenType::Stencil)
				msg += "'Stencil'";
			else if (token == ShaderLexer::TokenType::Layout)
				msg += "'Layout'";
			else if (token == ShaderLexer::TokenType::Uniform)
				msg += "'Uniform'";
			else if (token == ShaderLexer::TokenType::In)
				msg += "'In'";
			else if (token == ShaderLexer::TokenType::Out)
				msg += "'Out'";
		}
		msg += ')';
		return GetTokenErrorString(msg);
	}
	auto ShaderParser::GetTokenErrorString(const std::string& msg) const -> std::string
	{
		const ShaderLexer::Token& tk = PeekToken();
		return fmt::format("{} (line: {}, col: {})", msg, tk.line, tk.column);
	}

	auto ShaderParser::ParseShader() -> ShaderAST::ShaderNode
	{
		ShaderAST::ShaderNode shaderNode;
		
		while (CheckToken(ShaderLexer::TokenType::Preprocessor))
		{
			NextToken();
			ParsePreprocessor(shaderNode);
		}

		ConsumeToken(ShaderLexer::TokenType::Shader);

		if (auto& token = PeekToken(); token.type == ShaderLexer::TokenType::String)
		{
			shaderNode.shaderName = token.text;
			NextToken();
		}
		else
			throw ShaderParserException{ "Need to name a Shader!" };

		ConsumeToken(ShaderLexer::TokenType::LBrace);

		// 프로퍼티가 있으면 파싱 (없을 수도 있음)
		if (CheckToken(ShaderLexer::TokenType::Property))
			ParseProperty(shaderNode);

		while (!CheckToken(ShaderLexer::TokenType::RBrace) && !CheckToken(ShaderLexer::TokenType::EndOfFile))
		{
			shaderNode.passes.push_back(ParsePass(shaderNode));
		}

		ConsumeToken(ShaderLexer::TokenType::RBrace);
		return shaderNode;
	}
	void ShaderParser::ParsePreprocessor(ShaderAST::ShaderNode& shaderNode)
	{ 
		ConsumeToken(ShaderLexer::TokenType::Identifier);
		if (PreviousToken().text == "version")
		{
			ShaderAST::VersionNode versionNode{};
			ConsumeToken(ShaderLexer::TokenType::Number);
			versionNode.versionNumber = std::stoi(PreviousToken().text);
			ConsumeToken(ShaderLexer::TokenType::Identifier);
			versionNode.profile = PreviousToken().text;
			shaderNode.version = versionNode;
			return;
		}
		else
			throw ShaderParserException{ fmt::format("Unknown preprocessor identifier {} (line: {}, col: {})", PeekToken().text, PeekToken().line, PeekToken().column).c_str()};
	}
	void ShaderParser::ParseProperty(ShaderAST::ShaderNode& shaderNode)
	{
		ConsumeToken(ShaderLexer::TokenType::Property);
		ConsumeToken(ShaderLexer::TokenType::LBrace);

		while (!CheckToken(ShaderLexer::TokenType::RBrace))
		{
			ShaderAST::VariableNode varNode;
			varNode.attribute = ShaderAST::VariableAttribute::None;

			ConsumeToken({ ShaderLexer::TokenType::LSquareBracket, ShaderLexer::TokenType::Identifier, ShaderLexer::TokenType::Sampler2D });
			if (PreviousToken().type == ShaderLexer::TokenType::Identifier) // type
				varNode.type = IdentifierToVaraibleType(PreviousToken());
			else if(PreviousToken().type == ShaderLexer::TokenType::Sampler2D)
				varNode.type = ShaderAST::VariableType::Sampler;
			else // [Local]...
			{
				ConsumeToken(ShaderLexer::TokenType::Identifier);
				std::string attribute = PreviousToken().text;
				ConsumeToken(ShaderLexer::TokenType::RSquareBracket);
				if (attribute == "Local")
					varNode.attribute = ShaderAST::VariableAttribute::Local;
				else
					throw ShaderParserException(fmt::format("Unknown attribute: {} {}", attribute, GetCurrentTokenPosString()).c_str());
				ConsumeToken({ ShaderLexer::TokenType::Identifier, ShaderLexer::TokenType::Sampler2D }); // 타입
				if (PreviousToken().type == ShaderLexer::TokenType::Identifier)
					varNode.type = IdentifierToVaraibleType(PreviousToken());
				else
					varNode.type = ShaderAST::VariableType::Sampler;
			}

			ConsumeToken(ShaderLexer::TokenType::Identifier); // 변수 이름
			varNode.name = PreviousToken().text;
			ConsumeToken({ ShaderLexer::TokenType::Semicolon, ShaderLexer::TokenType::LSquareBracket });
			// 배열
			if (PreviousToken().type == ShaderLexer::TokenType::LSquareBracket)
			{
				ConsumeToken(ShaderLexer::TokenType::Number);
				varNode.size = std::stoi(PreviousToken().text);
				ConsumeToken(ShaderLexer::TokenType::RSquareBracket);
				ConsumeToken(ShaderLexer::TokenType::Semicolon);
			}
			shaderNode.properties.push_back(std::move(varNode));
		}

		ConsumeToken(ShaderLexer::TokenType::RBrace);
	}
	auto ShaderParser::ParsePass(const ShaderAST::ShaderNode& shaderNode) -> ShaderAST::PassNode
	{
		ShaderAST::PassNode passNode;
		passNode.name = fmt::format("Pass{}", passCount++);
		passNode.cullMode = CullMode::Back;

		ConsumeToken(ShaderLexer::TokenType::Pass);
		ConsumeToken({ ShaderLexer::TokenType::LBrace, ShaderLexer::TokenType::String }); // 새 패스 시작
		lastObjectUniformBinding = 0;
		lastMaterialUniformBinding = 0;
		if (PreviousToken().type == ShaderLexer::TokenType::String)
		{
			passNode.name = PreviousToken().text;
			ConsumeToken(ShaderLexer::TokenType::LBrace);
		}

		while (!CheckToken(ShaderLexer::TokenType::RBrace) && !CheckToken(ShaderLexer::TokenType::EndOfFile))
		{
			if (CheckToken(ShaderLexer::TokenType::LightingPass))
				ParseLightingPass(passNode);
			else if (CheckToken(ShaderLexer::TokenType::Stencil))
				ParseStencil(passNode);
			else if (CheckToken(ShaderLexer::TokenType::Cull))
				ParseCull(passNode);
			else if (CheckToken(ShaderLexer::TokenType::ZWrite))
				ParseZWrite(passNode);
			else if (CheckToken(ShaderLexer::TokenType::ColorMask))
				ParseColorMask(passNode);
			else
				passNode.stages.push_back(ParseStage(shaderNode, passNode));
		}

		ConsumeToken(ShaderLexer::TokenType::RBrace);
		return passNode;
	}
	void ShaderParser::ParseLightingPass(ShaderAST::PassNode& passNode)
	{
		if (!CheckToken(ShaderLexer::TokenType::LightingPass))
			return;
		NextToken();

		ConsumeToken(ShaderLexer::TokenType::String);
		passNode.lightingPass = PreviousToken().text;
	}
	void ShaderParser::ParseStencil(ShaderAST::PassNode& passNode)
	{
		if (!CheckToken(ShaderLexer::TokenType::Stencil))
			return;

		ConsumeToken(ShaderLexer::TokenType::Stencil);
		ConsumeToken(ShaderLexer::TokenType::LBrace);
		while (!CheckToken(ShaderLexer::TokenType::RBrace))
		{
			ConsumeToken({ ShaderLexer::TokenType::Identifier, ShaderLexer::TokenType::Pass });
			{
				//Comp Always;
				//Pass Keep;
				//Fail Keep;
				//ZFail Keep;
				auto& token = PreviousToken().text;
				if (token == "Ref" || token == "ReadMask" || token == "WriteMask")
				{
					ConsumeToken(ShaderLexer::TokenType::Number);
					if (token == "Ref")
						passNode.stencil.ref = std::stoi(PreviousToken().text);
					else if (token == "ReadMask")
						passNode.stencil.compareMask = std::stoi(PreviousToken().text);
					else if (token == "WriteMask")
						passNode.stencil.writeMask = std::stoi(PreviousToken().text);
				}
				else if (token == "Comp")
				{
					ConsumeToken(ShaderLexer::TokenType::Identifier);
					std::array<const char*, 8> compTokens = { "Never", "Less", "Equal", "LessEqual", "Greater", "NotEqual", "GreaterEqual", "Always" };
					const char* compToken = PreviousToken().text.c_str();
					auto it = std::find_if(compTokens.begin(), compTokens.end(), [&](const char* s) { return std::strcmp(s, compToken) == 0; });
					if (it == compTokens.end())
					{
						std::string err = "Allowed identifiers: ";
						for (int i = 0; i < compTokens.size(); ++i)
						{
							err += compTokens[i];
							if (i != compTokens.size() - 1)
								err += ", ";
						}
						throw ShaderParserException{ GetTokenErrorString(err).c_str() };
					}
					else
					{
						int idx = static_cast<int>(std::distance(compTokens.begin(), it));
						passNode.stencil.compareOp = static_cast<StencilState::CompareOp>(idx);
					}
				}
				else if (token == "Pass" || token == "Fail" || token == "ZFail")
				{
					ConsumeToken(ShaderLexer::TokenType::Identifier);
					std::array<const char*, 8> stencilTokens = { "Keep", "Zero", "Replace", "IncrementClamp", "DecrementClamp", "Invert", "IncrementWrap", "DecrementWrap" };
					const char* compToken = PreviousToken().text.c_str();
					auto it = std::find_if(stencilTokens.begin(), stencilTokens.end(), [&](const char* s) { return std::strcmp(s, compToken) == 0; });
					if (it == stencilTokens.end())
					{
						std::string err = "Allowed identifiers: ";
						for (int i = 0; i < stencilTokens.size(); ++i)
						{
							err += stencilTokens[i];
							if (i != stencilTokens.size() - 1)
								err += ", ";
						}
						throw ShaderParserException{ GetTokenErrorString(err).c_str() };
					}
					else
					{
						int idx = static_cast<int>(std::distance(stencilTokens.begin(), it));
						if (token == "Pass")
							passNode.stencil.passOp = static_cast<StencilState::StencilOp>(idx);
						else if (token == "Fail")
							passNode.stencil.failOp = static_cast<StencilState::StencilOp>(idx);
						else if (token == "ZFail")
							passNode.stencil.depthFailOp = static_cast<StencilState::StencilOp>(idx);
					}
				}
				else
				{
					std::string err = "Allowed identifiers: Ref, ReadMask, WriteMask, Comp, Pass, Fail, ZFail";
					throw ShaderParserException{ GetTokenErrorString(err).c_str() };
				}
				ConsumeToken(ShaderLexer::TokenType::Semicolon);
			}
		}
		ConsumeToken(ShaderLexer::TokenType::RBrace);
	}
	void ShaderParser::ParseCull(ShaderAST::PassNode& passNode)
	{
		ConsumeToken(ShaderLexer::TokenType::Cull);
		ConsumeToken(ShaderLexer::TokenType::Identifier);
		const std::string& ident = PreviousToken().text;
		if (ident == "Off" || ident == "off")
			passNode.cullMode = CullMode::Off;
		else if (ident == "Front" || ident == "front")
			passNode.cullMode = CullMode::Front;
		else if (ident == "Back" || ident == "back")
			passNode.cullMode = CullMode::Back;
		else
			throw ShaderParserException{"Allowed Cull identifiers: Off, Front, Back"};
		ConsumeToken(ShaderLexer::TokenType::Semicolon);
	}
	void ShaderParser::ParseZWrite(ShaderAST::PassNode& passNode)
	{
		ConsumeToken(ShaderLexer::TokenType::ZWrite);
		ConsumeToken(ShaderLexer::TokenType::Identifier);
		const std::string& ident = PreviousToken().text;
		if (ident == "Off" || ident == "off")
			passNode.zwrite = false;
		else if (ident == "On" || ident == "on")
			passNode.zwrite = true;
		else
			throw ShaderParserException{ "Allowed ZWrite identifiers: Off, On" };
		ConsumeToken(ShaderLexer::TokenType::Semicolon);
	}
	void ShaderParser::ParseColorMask(ShaderAST::PassNode& passNode)
	{
		ConsumeToken(ShaderLexer::TokenType::ColorMask);
		ConsumeToken({ ShaderLexer::TokenType::Identifier, ShaderLexer::TokenType::Number });
		auto& prevToken = PreviousToken();
		passNode.colorMask = 0;
		if (prevToken.type != ShaderLexer::TokenType::Number)
		{
			bool allow = false;
			if (prevToken.text.find("R") != prevToken.text.npos)
			{
				passNode.colorMask |= 1;
				allow = true;
			}
			if(prevToken.text.find("G") != prevToken.text.npos)
			{
				passNode.colorMask |= 2;
				allow = true;
			}
			if (prevToken.text.find("B") != prevToken.text.npos)
			{
				passNode.colorMask |= 4;
				allow = true;
			}
			if (prevToken.text.find("A") != prevToken.text.npos)
			{
				passNode.colorMask |= 8;
				allow = true;
			}
			if (!allow)
				throw ShaderParserException{ "Allowed ZWrite identifiers: R, G, B, A" };
		}
		ConsumeToken(ShaderLexer::TokenType::Semicolon);
	}
	auto ShaderParser::ParseStage(const ShaderAST::ShaderNode& shaderNode, ShaderAST::PassNode& passNode) -> ShaderAST::StageNode
	{
		ShaderAST::StageNode stage;
		ConsumeToken(ShaderLexer::TokenType::Stage);

		if (CheckToken(ShaderLexer::TokenType::Vertex))
		{
			stage.type = ShaderAST::StageType::Vertex;
			NextToken();
		}
		else if (CheckToken(ShaderLexer::TokenType::Fragment))
		{
			stage.type = ShaderAST::StageType::Fragment;
			NextToken();
		}
		else
		{
			stage.type = ShaderAST::StageType::Unknown;
			// 식별자일 수도 있고 다른 키워드일 수도 있으니 일단 next
			NextToken();
		}

		ConsumeToken(ShaderLexer::TokenType::LBrace);
		ParseStageBody(shaderNode, stage, passNode);
		ConsumeToken(ShaderLexer::TokenType::RBrace);

		return stage;
	}
	void ShaderParser::ParseStageBody(const ShaderAST::ShaderNode& shaderNode, ShaderAST::StageNode& stageNode, ShaderAST::PassNode& passNode)
	{
		// 이미 '{'는 읽은 뒤라 nesting = 1 로 시작
		int nesting = 1;
		while (!CheckToken(ShaderLexer::TokenType::EndOfFile))
		{
			if (CheckToken(ShaderLexer::TokenType::LBrace)) // {
			{
				NextToken();
				nesting++;
			}
			else if (CheckToken(ShaderLexer::TokenType::RBrace)) // }
			{
				nesting--;
				if (nesting == 0) // Stage 끝
					break;
				else
					NextToken();
			}
			else if (CheckToken(ShaderLexer::TokenType::Layout))
				ParseLayout(stageNode);
			else if (CheckToken(ShaderLexer::TokenType::Uniform))
				ParseUniform(shaderNode, stageNode);
			else if (CheckToken(ShaderLexer::TokenType::Const))
			{
				NextToken();
				ParseDeclaration(stageNode, "const");
			}
			else if (CheckToken(ShaderLexer::TokenType::Constexpr))
				ParseConstexpr(passNode);
			else if (CheckToken(ShaderLexer::TokenType::Identifier))
			{
				ParseDeclaration(stageNode);
			}
			else
				NextToken();
		}
	}
	void ShaderParser::ParseDeclaration(ShaderAST::StageNode& stageNode, const std::string& qualifer)
	{
		// <정의> ::= <ident> <ident> <rest>
		// <rest> ::= ';' | '=' <expr> ';' | '(' <parameters> ')' <function body>
		ConsumeToken(ShaderLexer::TokenType::Identifier);
		const std::string& type = PreviousToken().text;
		ConsumeToken(ShaderLexer::TokenType::Identifier);
		const std::string& name = PreviousToken().text;
		bool array = false;
		std::size_t arraySize = 0;
		if (PeekToken().type == ShaderLexer::TokenType::LSquareBracket)
		{
			NextToken();
			ConsumeToken(ShaderLexer::TokenType::Number);
			arraySize = std::stoi(PreviousToken().text);
			ConsumeToken(ShaderLexer::TokenType::RSquareBracket);
			array = true;
		}
		ConsumeToken({ ShaderLexer::TokenType::Semicolon, ShaderLexer::TokenType::Operator, ShaderLexer::TokenType::LBracket});
		auto& prevToken = PreviousToken();
		if (prevToken.type == ShaderLexer::TokenType::Semicolon) // 변수 선언
		{
			if (!array)
				stageNode.declaration.push_back(fmt::format("{} {} {};", qualifer, type, name));
			else
				stageNode.declaration.push_back(fmt::format("{} {} {}[{}];", qualifer, type, name, arraySize));
		}
		else if (prevToken.type == ShaderLexer::TokenType::Operator) // 변수 선언과 정의
		{
			if (prevToken.text != "=")
				throw ShaderParserException{ GetTokenErrorString("Not found =").c_str() };
			std::string expr;
			while (!CheckToken(ShaderLexer::TokenType::Semicolon))
			{
				if (PeekToken().type == ShaderLexer::TokenType::Literal)
					expr.pop_back();
				expr += PeekToken().text;
				expr += " ";
				NextToken();
			}
			NextToken(); // ;
			if (!array)
				stageNode.declaration.push_back(fmt::format("{} {} {} = {};", qualifer, type, name, std::move(expr)));
			else
				stageNode.declaration.push_back(fmt::format("{} {} {}[{}] = {};", qualifer, type, name, arraySize, std::move(expr)));
		}
		else if (prevToken.type == ShaderLexer::TokenType::LBracket) // 함수 정의
		{
			std::string parameters;
			while (!CheckToken(ShaderLexer::TokenType::RBracket))
			{
				if (PeekToken().type == ShaderLexer::TokenType::Literal)
					parameters.pop_back();
				parameters += PeekToken().text;
				parameters += " ";
				NextToken();
			}
			ConsumeToken(ShaderLexer::TokenType::RBracket);
			ConsumeToken(ShaderLexer::TokenType::LBrace);
			std::string functionBody{ ParseFunctionBody(stageNode) };
			ConsumeToken(ShaderLexer::TokenType::RBrace);
			stageNode.functions.push_back(fmt::format("{} {} {}({}) {{ {} }}", qualifer, type, name, std::move(parameters), std::move(functionBody)));
		}
	}

	auto ShaderParser::ParseFunctionBody(ShaderAST::StageNode& stageNode) -> std::string
	{
		auto uboit = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
		{
			return ubo.name == "UBO";
		});

		std::string code{};
		int nested = 1;
		bool usingVertex = false;
		bool usingNormal = false;
		bool usingUV = false;
		bool usingTangent = false;
		bool usingMatrixModel = false;
		bool usingCamera = false;
		bool usingLIGHT = false;
		while (nested != 0 || PeekToken().type != ShaderLexer::TokenType::EndOfFile)
		{
			auto& token = PeekToken();
			if (token.type == ShaderLexer::TokenType::RBrace)
			{
				--nested;
				if (nested == 0)
					break;
			}
			else if (token.type == ShaderLexer::TokenType::LBrace)
				++nested;
			else if (token.type == ShaderLexer::TokenType::Literal)
			{
				if (!code.empty())
					code.pop_back();
			}
			else if (CheckToken(ShaderLexer::TokenType::VERTEX))
			{
				if (!usingVertex)
				{
					auto it = std::find_if(stageNode.in.begin(), stageNode.in.end(), [&](const ShaderAST::LayoutNode& layoutNode)
						{
							return layoutNode.var.name == "VERTEX";
						});
					if (it == stageNode.in.end())
					{
						ShaderAST::LayoutNode layoutNode{};
						layoutNode.binding = Mesh::VERTEX_ID;
						layoutNode.var.name = "VERTEX";
						layoutNode.var.type = ShaderAST::VariableType::Vec3;
						layoutNode.var.size = 1;
						stageNode.in.push_back(std::move(layoutNode));
						usingVertex = true;
					}
					else
						usingVertex = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::UV))
			{
				if (!usingUV)
				{
					auto it = std::find_if(stageNode.in.begin(), stageNode.in.end(), [&](const ShaderAST::LayoutNode& layoutNode)
							{
								return layoutNode.var.name == "UV";
							});
					if (it == stageNode.in.end())
					{
						ShaderAST::LayoutNode layoutNode{};
						layoutNode.binding = Mesh::UV_ID;
						layoutNode.var.name = "UV";
						layoutNode.var.type = ShaderAST::VariableType::Vec2;
						layoutNode.var.size = 1;
						stageNode.in.push_back(std::move(layoutNode));
						usingUV = true;
					}
					else
						usingUV = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::NORMAL))
			{
				if (!usingNormal)
				{
					auto it = std::find_if(stageNode.in.begin(), stageNode.in.end(), [&](const ShaderAST::LayoutNode& layoutNode)
					{
						return layoutNode.var.name == "NORMAL";
					});
					if (it == stageNode.in.end())
					{
						ShaderAST::LayoutNode layoutNode{};
						layoutNode.binding = Mesh::NORMAL_ID;
						layoutNode.var.name = "NORMAL";
						layoutNode.var.type = ShaderAST::VariableType::Vec3;
						layoutNode.var.size = 1;
						stageNode.in.push_back(std::move(layoutNode));
						usingNormal = true;
					}
					else
						usingNormal = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::TANGENT))
			{
				if (!usingTangent)
				{
					auto it = std::find_if(stageNode.in.begin(), stageNode.in.end(), [&](const ShaderAST::LayoutNode& layoutNode)
					{
						return layoutNode.var.name == "TANGENT";
					});
					if (it == stageNode.in.end())
					{
						ShaderAST::LayoutNode layoutNode{};
						layoutNode.binding = Mesh::TANGENT_ID;
						layoutNode.var.name = "TANGENT";
						layoutNode.var.type = ShaderAST::VariableType::Vec3;
						layoutNode.var.size = 1;
						stageNode.in.push_back(std::move(layoutNode));
						usingTangent = true;
					}
					else
						usingTangent = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::MATRIX_VIEW) || CheckToken(ShaderLexer::TokenType::MATRIX_PROJ))
			{
				if (!usingCamera)
				{
					auto it = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
					{
						return ubo.name == "CAMERA";
					});
					if (it == stageNode.uniforms.end())
					{
						ShaderAST::UBONode uboNode{};
						uboNode.name = "CAMERA";
						uboNode.set = static_cast<uint32_t>(UniformStructLayout::Type::Camera);
						uboNode.binding = 0;
						uboNode.bSampler = false;
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Mat4, 1, "view" });
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Mat4, 1, "proj" });
						stageNode.uniforms.push_back(std::move(uboNode));
						uboit = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
						{
							return ubo.name == "UBO";
						});
					}
					usingCamera = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::MATRIX_MODEL))
			{
				if (!usingMatrixModel)
				{
					auto it = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
					{
						return ubo.name == "CONSTANTS";
					});
					if (it == stageNode.uniforms.end())
					{
						ShaderAST::UBONode uboNode{};
						uboNode.name = "CONSTANTS";
						uboNode.set = static_cast<uint32_t>(UniformStructLayout::Type::Object); // 의미 없음
						uboNode.binding = 0; // 의미 없음
						uboNode.bConstant = true;
						uboNode.bSampler = false;
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Mat4, 1, "model" });
						stageNode.uniforms.push_back(std::move(uboNode));
						uboit = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
						{
							return ubo.name == "UBO";
						});
					}
					usingMatrixModel = true;
				}
			}
			else if (CheckToken(ShaderLexer::TokenType::LIGHT))
			{
				if (!usingLIGHT)
				{
					auto it = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
					{
						return ubo.name == "LIGHT";
					});
					if (it == stageNode.uniforms.end())
					{
						ShaderAST::UBONode uboNode{};
						uboNode.name = "LIGHT";
						uboNode.set = static_cast<uint32_t>(UniformStructLayout::Type::Object);
						uboNode.binding = lastObjectUniformBinding++;
						uboNode.bSampler = false;
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Int, 1, "count" });
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Vec4, 10, "pos" });
						uboNode.vars.push_back(ShaderAST::VariableNode{ ShaderAST::VariableType::Vec4, 10, "other" });
						stageNode.uniforms.push_back(std::move(uboNode));
						stageNode.bUseLighting = true;
						usingLIGHT = true;
						uboit = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& ubo)
						{
							return ubo.name == "UBO";
						});
					}
				}
			}
			else if (token.type == ShaderLexer::TokenType::Identifier)
			{
				if (uboit != stageNode.uniforms.end())
				{
					auto varit = std::find_if(uboit->vars.begin(), uboit->vars.end(), [&](const ShaderAST::VariableNode& var)
					{
						return var.name == token.text;
					});
					if (varit != uboit->vars.end())
						code += "UBO.";
				}
			}
			code += SubstitutionFunctionToken(token) + " ";
			NextToken();
		}
		return code;
	}

	void ShaderParser::ParseLayout(ShaderAST::StageNode& stageNode)
	{
		ConsumeToken(ShaderLexer::TokenType::Layout);
		ShaderAST::LayoutNode layoutNode{};
		ShaderAST::UBONode uboNode{};
		ConsumeToken(ShaderLexer::TokenType::LBracket); // (

		ConsumeToken(ShaderLexer::TokenType::Identifier);

		bool bUniform = false;
		if (PreviousToken().text == "location") // 어트리뷰트
		{
			ConsumeToken(ShaderLexer::TokenType::Operator);
			ConsumeToken(ShaderLexer::TokenType::Number);
			layoutNode.binding = std::stoi(PreviousToken().text);
		}
		else if (PreviousToken().text == "set") // 유니폼
		{
			bUniform = true;
			ConsumeToken(ShaderLexer::TokenType::Operator);
			ConsumeToken(ShaderLexer::TokenType::Number);
			uboNode.set = std::stoi(PreviousToken().text);
			ConsumeToken(ShaderLexer::TokenType::Unknown); // ','
			ConsumeToken(ShaderLexer::TokenType::Identifier);
			if (PreviousToken().text == "binding")
			{
				ConsumeToken(ShaderLexer::TokenType::Operator);
				ConsumeToken(ShaderLexer::TokenType::Number);
				uboNode.binding = std::stoi(PreviousToken().text);
			}
			else
				throw ShaderParserException{ GetTokenErrorString("Not found binding token").c_str() };
		}
		ConsumeToken(ShaderLexer::TokenType::RBracket); // )

		if (!bUniform) // 어트리뷰트 in, out 처리
		{
			auto& inoutToken = PeekToken();
			if (CheckToken(ShaderLexer::TokenType::In) || CheckToken(ShaderLexer::TokenType::Out))
			{
				NextToken();
				ShaderAST::VariableNode varNode{};
				ConsumeToken(ShaderLexer::TokenType::Identifier);
				varNode.type = IdentifierToVaraibleType(PreviousToken());
				ConsumeToken(ShaderLexer::TokenType::Identifier);
				varNode.name = PreviousToken().text;

				layoutNode.var = std::move(varNode);
				if (inoutToken.type == ShaderLexer::TokenType::In)
					stageNode.in.push_back(layoutNode);
				else
					stageNode.out.push_back(layoutNode);
			}
			else
				throw ShaderParserException{ GetTokenErrorString("Not found 'in' or 'out' keyword").c_str() };
		}
		else // Uniform
		{
			ConsumeToken(ShaderLexer::TokenType::Uniform);
			if (CheckToken(ShaderLexer::TokenType::Identifier))
			{
				NextToken();
				ParseUniformBody(uboNode);
				ConsumeToken(ShaderLexer::TokenType::Identifier);
				uboNode.name = PreviousToken().text;
				ConsumeToken(ShaderLexer::TokenType::Semicolon);
			}
			else if (CheckToken(ShaderLexer::TokenType::Sampler2D))
			{
				uboNode.bSampler = true;
				NextToken();
				ConsumeToken(ShaderLexer::TokenType::Identifier);
				uboNode.name = PreviousToken().text;
				ConsumeToken(ShaderLexer::TokenType::Semicolon);
			}
			else
				throw ShaderParserException({ GetTokenErrorString("Not found Identifier or Sampler2D keyword").c_str()});

			stageNode.uniforms.push_back(std::move(uboNode));
		}
	}
	void ShaderParser::ParseUniform(const ShaderAST::ShaderNode& shaderNode, ShaderAST::StageNode& stageNode)
	{
		ConsumeToken(ShaderLexer::TokenType::Uniform);
		ConsumeToken({ ShaderLexer::TokenType::Identifier, ShaderLexer::TokenType::Sampler2D });

		ShaderAST::VariableNode varNode{};
		std::string varType;
		if (PreviousToken().type == ShaderLexer::TokenType::Identifier)
		{
			varType = PreviousToken().text;
			varNode.type = IdentifierToVaraibleType(PreviousToken());
		}
		else
		{
			varType = "Sampler2D";
			varNode.type = ShaderAST::VariableType::Sampler;
		}

		ConsumeToken(ShaderLexer::TokenType::Identifier);
		varNode.name = PreviousToken().text;

		ConsumeToken({ ShaderLexer::TokenType::Semicolon, ShaderLexer::TokenType::LSquareBracket });
		// 배열
		if (PreviousToken().type == ShaderLexer::TokenType::LSquareBracket)
		{
			ConsumeToken(ShaderLexer::TokenType::Number);
			varNode.size = std::stoi(PreviousToken().text);
			ConsumeToken(ShaderLexer::TokenType::RSquareBracket);
			ConsumeToken(ShaderLexer::TokenType::Semicolon);
		}

		// 프로퍼티 구문에 있는지 검사
		bool hasProperty = false;
		uint32_t set = static_cast<uint32_t>(UniformStructLayout::Type::Material);
		for (auto& property : shaderNode.properties)
		{
			if (property.type == varNode.type && property.size == varNode.size && property.name == varNode.name)
			{
				set = (property.attribute == ShaderAST::VariableAttribute::Local) ? 
					static_cast<uint32_t>(UniformStructLayout::Type::Object) : static_cast<uint32_t>(UniformStructLayout::Type::Material);
				hasProperty = true;
				break;
			}
		}
		if (!hasProperty)
			throw ShaderParserException(fmt::format("Variable({} {}) is not declared in property", varType, varNode.name).c_str());

		if (varNode.type != ShaderAST::VariableType::Sampler)
		{
			auto it = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& uboNode)
				{
					return uboNode.name == "UBO";
				}
			);
			if (it == stageNode.uniforms.end())
			{
				ShaderAST::UBONode uboNode{};
				uboNode.set = set;
				uboNode.binding = lastMaterialUniformBinding++;
				uboNode.name = "UBO";
				uboNode.bSampler = false;
				uboNode.vars.push_back(std::move(varNode));
				stageNode.uniforms.push_back(std::move(uboNode));
			}
			else
			{
				ShaderAST::UBONode& uboNode = *it;
				uboNode.vars.push_back(std::move(varNode));
			}
		}
		else
		{
			ShaderAST::UBONode uboNode{};
			uboNode.bSampler = true;
			uboNode.name = varNode.name;
			uboNode.set = set;
			uboNode.binding = (set == static_cast<uint32_t>(ShaderAST::VariableAttribute::Local)) ? 
				lastObjectUniformBinding++ : lastMaterialUniformBinding++;
			stageNode.uniforms.push_back(std::move(uboNode));
		}
	}
	void ShaderParser::ParseUniformBody(ShaderAST::UBONode& uboNode)
	{
		ConsumeToken(ShaderLexer::TokenType::LBrace); // {

		while (!CheckToken(ShaderLexer::TokenType::RBrace))
		{
			ShaderAST::VariableNode varNode{};
			ConsumeToken(ShaderLexer::TokenType::Identifier);
			varNode.type = IdentifierToVaraibleType(PreviousToken());
			ConsumeToken(ShaderLexer::TokenType::Identifier);
			varNode.name = PreviousToken().text;
			if (CheckToken(ShaderLexer::TokenType::LSquareBracket)) // 배열
			{
				NextToken();
				ConsumeToken(ShaderLexer::TokenType::Number);
				varNode.size = std::stoi(PreviousToken().text);
				ConsumeToken(ShaderLexer::TokenType::RSquareBracket);
			}
			ConsumeToken(ShaderLexer::TokenType::Semicolon);

			uboNode.vars.push_back(std::move(varNode));
		}

		ConsumeToken(ShaderLexer::TokenType::RBrace); // }
	}
	void ShaderParser::ParseConstexpr(ShaderAST::PassNode& passNode)
	{
		// constexpr idt idt;
		ConsumeToken(ShaderLexer::TokenType::Constexpr);
		ConsumeToken(ShaderLexer::TokenType::Identifier);

		ShaderAST::VariableNode varNode;
		varNode.type = IdentifierToVaraibleType(PreviousToken());

		ConsumeToken(ShaderLexer::TokenType::Identifier);
		varNode.name = PreviousToken().text;
		ConsumeToken({ ShaderLexer::TokenType::Semicolon, ShaderLexer::TokenType::Operator });
		// constexpr idt idt = number | idt;
		if (PreviousToken().type == ShaderLexer::TokenType::Operator && PreviousToken().text == "=")
		{
			ConsumeToken({ ShaderLexer::TokenType::Number, ShaderLexer::TokenType::Identifier });
			varNode.defaultValue = PreviousToken().text;
			ConsumeToken(ShaderLexer::TokenType::Semicolon);
		}
		// 중복 검사
		for (auto& constant : passNode.constants)
		{
			if (constant.name == varNode.name)
				return;
		}
		passNode.constants.push_back(std::move(varNode));
	}

	void ShaderParser::Optimize(ShaderAST::ShaderNode& shaderNode)
	{
		for (auto& passNode : shaderNode.passes)
		{
			for (auto& stageNode : passNode.stages)
			{
				// UBO 유니폼 변수 정렬 (레이아웃 재배치)
				auto it = std::find_if(stageNode.uniforms.begin(), stageNode.uniforms.end(), [&](const ShaderAST::UBONode& uboNode)
				{
					return uboNode.name == "UBO";
				});
				if (it == stageNode.uniforms.end())
					continue;

				ShaderAST::UBONode& uboNode = *it;

				std::sort(uboNode.vars.begin(), uboNode.vars.end(), [&](const ShaderAST::VariableNode& varNode1, const ShaderAST::VariableNode& varNode2)
				{
					bool scalarA = varNode1.type == ShaderAST::VariableType::Int || varNode1.type == ShaderAST::VariableType::Float;
					bool scalarB = varNode2.type == ShaderAST::VariableType::Int || varNode2.type == ShaderAST::VariableType::Float;
					bool vec2A = varNode1.type == ShaderAST::VariableType::Vec2;
					bool vec2B = varNode2.type == ShaderAST::VariableType::Vec2;
					bool bigA = !scalarA && !vec2A;
					bool bigB = !scalarB && !vec2B;
					if (scalarA && !scalarB)
						return true;
					if (vec2A && bigB)
						return true;
					return false;
				});
			}
		}
	}

	void ShaderParser::GenerateStageCode(int stageIdx, const ShaderAST::ShaderNode& shaderNode, const ShaderAST::PassNode& passNode, ShaderAST::StageNode& stageNode)
	{
		std::string code = fmt::format("#version {} {}\n", shaderNode.version.versionNumber, shaderNode.version.profile);
		for (auto& in : stageNode.in)
			code += fmt::format("layout(location = {}) in {} {};\n", in.binding, VariableTypeToString(in.var.type), in.var.name);
		for (auto& out : stageNode.out)
			code += fmt::format("layout(location = {}) out {} {};\n", out.binding, VariableTypeToString(out.var.type), out.var.name);

		for (auto& uniform : stageNode.uniforms)
		{
			if (uniform.bSampler)
			{
				code += fmt::format("layout(set = {}, binding = {}) uniform sampler2D {};\n", uniform.set, uniform.binding, uniform.name);
				continue;
			}
			std::string uniformMembers;
			for (auto& member : uniform.vars)
			{
				if (member.size == 1)
					uniformMembers += fmt::format("{} {};\n", VariableTypeToString(member.type), member.name);
				else
					uniformMembers += fmt::format("{} {}[{}];\n", VariableTypeToString(member.type), member.name, member.size);
			}
			if (uniform.bConstant)
			{
				code += fmt::format("layout(push_constant) uniform UNIFORM_{} {{\n {} }} {};\n", uniform.name, uniformMembers, uniform.name);
				continue;
			}
			code += fmt::format("layout(set = {}, binding = {}) uniform {} {{\n {} }} {};\n", uniform.set, uniform.binding, "UNIFORM_" + uniform.name, uniformMembers, uniform.name);
		}
		for (int i = 0; i < passNode.constants.size(); ++i)
		{
			const auto& varNode = passNode.constants[i];
			if (varNode.defaultValue.empty())
				code += fmt::format("layout (constant_id = {}) const {} {};", i, VariableTypeToString(varNode.type), varNode.name);
			else
				code += fmt::format("layout (constant_id = {}) const {} {} = {};", i, VariableTypeToString(varNode.type), varNode.name, varNode.defaultValue);
		}
		for (auto& decl : stageNode.declaration)
			code += decl + '\n';
		for (auto& function : stageNode.functions)
			code += function + '\n';
		stageNode.code = std::move(code);
	}
	auto ShaderParser::SubstitutionFunctionToken(const ShaderLexer::Token& token) const -> std::string
	{
		static const std::unordered_map<std::string, std::string> replaceMap =
		{
			{"MATRIX_MODEL", "CONSTANTS.model"},
			{"MATRIX_VIEW", "CAMERA.view"},
			{"MATRIX_PROJ", "CAMERA.proj"},
		};
		auto it = replaceMap.find(token.text);
		if (it == replaceMap.end())
			return token.text;
		return it->second;
	}

	SH_RENDER_API auto sh::render::ShaderParser::Parse(const std::vector<ShaderLexer::Token>& tokens) -> ShaderAST::ShaderNode
	{
		this->tokens = &tokens;
		pos = 0;
		lastObjectUniformBinding = 0;
		lastMaterialUniformBinding = 0;

		ShaderAST::ShaderNode shaderNode = ParseShader();
		Optimize(shaderNode);
		int idx = 0;
		for (auto& passNode : shaderNode.passes)
		{
			for (auto& stageNode : passNode.stages)
			{
				GenerateStageCode(idx++, shaderNode, passNode, stageNode);
			}
		}
		return shaderNode;
	}
}//namespace