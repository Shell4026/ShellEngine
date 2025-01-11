#include "ShaderParser.h"
#include "StencilState.h"

#include <fmt/core.h>

#include <cctype>
#include <array>
#include <algorithm>

namespace sh::render
{
	auto ShaderParser::IsIdentifierChar(char c) -> bool
	{
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}
	auto ShaderParser::PeekToken() const -> const Token&
	{
		return tokens[pos];
	}
	auto ShaderParser::CheckToken(TokenType token) const -> bool
	{
		return (PeekToken().type == token);
	}
	auto ShaderParser::NextToken() -> const Token&
	{
		if (pos < tokens.size())
			return tokens[pos++];
		return tokens[pos];
	}
	auto ShaderParser::PreviousToken() -> const Token&
	{
		return tokens[pos - 1];
	}
	void ShaderParser::ConsumeToken(TokenType token)
	{
		if (CheckToken(token))
			NextToken();
		else
		{
			const Token& tk = PeekToken();
			throw ShaderParserException{ GetNotFoundTokenString({ token }).c_str() };
		}
	}
	void ShaderParser::ConsumeToken(const std::initializer_list<TokenType>& tokens)
	{
		for (auto token : tokens)
		{
			if (CheckToken(token))
			{
				NextToken();
				return;
			}
		}
		const Token& tk = PeekToken();
		throw ShaderParserException{ GetNotFoundTokenString(tokens).c_str() };
	}

	auto ShaderParser::IdentifierToVaraibleType(const Token& token) const -> ShaderAST::VariableType
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
		if (type == "int")
			return ShaderAST::VariableType::Int;
		if (type == "float")
			return ShaderAST::VariableType::Float;
		throw ShaderParserException{ GetTokenErrorString("Not found valid variable token").c_str() };
	}
	auto ShaderParser::GetNotFoundTokenString(const std::initializer_list<TokenType>& tokens) -> std::string
	{
		std::string msg = "Not found token (";
		for (auto token : tokens)
		{
			if (token == TokenType::LBracket)
				msg += "'{'";
			else if (token == TokenType::RBracket)
				msg += "'}'";
			else if (token == TokenType::LBrace)
				msg += "'('";
			else if (token == TokenType::RBrace)
				msg += "')'";
			else if (token == TokenType::Pass)
				msg += "Pass";
			else if (token == TokenType::Stage)
				msg += "'Stage'";
			else if (token == TokenType::Stencil)
				msg += "'Stencil'";
			else if (token == TokenType::Layout)
				msg += "'Layout'";
			else if (token == TokenType::Uniform)
				msg += "'Uniform'";
			else if (token == TokenType::In)
				msg += "'In'";
			else if (token == TokenType::Out)
				msg += "'Out'";
		}
		msg += ')';
		return GetTokenErrorString(msg);
	}
	auto ShaderParser::GetTokenErrorString(const std::string& msg) const -> std::string
	{
		const Token& tk = PeekToken();
		return fmt::format("{} (line: {}, col: {})", msg, tk.line, tk.column);
	}

	auto ShaderParser::ParseShader() -> ShaderAST::ShaderNode
	{
		ShaderAST::ShaderNode shaderNode;
		
		while (CheckToken(TokenType::Preprocessor))
		{
			NextToken();
			ParsePreprocessor(shaderNode);
		}

		ConsumeToken(TokenType::Shader);

		if (auto& token = PeekToken(); token.type == TokenType::String)
		{
			shaderNode.shaderName = token.text;
			NextToken();
		}
		else
			throw ShaderParserException{ "Need to name a Shader!" };

		ConsumeToken(TokenType::LBrace);

		while (!CheckToken(TokenType::RBrace) && !CheckToken(TokenType::EndOfFile))
		{
			shaderNode.passes.push_back(ParsePass());
		}

		ConsumeToken(TokenType::RBrace);
		return shaderNode;
	}
	void ShaderParser::ParsePreprocessor(ShaderAST::ShaderNode& shaderNode)
	{ 
		ConsumeToken(TokenType::Identifier);
		if (PreviousToken().text == "version")
		{
			ShaderAST::VersionNode versionNode{};
			ConsumeToken(TokenType::Number);
			versionNode.versionNumber = std::stoi(PreviousToken().text);
			ConsumeToken(TokenType::Identifier);
			versionNode.profile = PreviousToken().text;
			shaderNode.version = versionNode;
			return;
		}
		else
			throw ShaderParserException{ fmt::format("Unknown preprocessor identifier {} (line: {}, col: {})", PeekToken().text, PeekToken().line, PeekToken().column).c_str()};
	}
	auto ShaderParser::ParsePass() -> ShaderAST::PassNode
	{
		ShaderAST::PassNode passNode;
		ConsumeToken(TokenType::Pass);
		ConsumeToken({ TokenType::LBrace, TokenType::String });
		if (PreviousToken().type == TokenType::String)
		{
			passNode.name = PreviousToken().text;
			ConsumeToken(TokenType::LBrace);
		}

		while (!CheckToken(TokenType::RBrace) && !CheckToken(TokenType::EndOfFile))
		{
			ParseStencil(passNode);
			passNode.stages.push_back(ParseStage());
		}

		ConsumeToken(TokenType::RBrace);
		return passNode;
	}
	void ShaderParser::ParseStencil(ShaderAST::PassNode& passNode)
	{
		ShaderAST::StencilNode stencilNode{};
		if (!CheckToken(TokenType::Stencil))
			return;

		ConsumeToken(TokenType::Stencil);
		ConsumeToken(TokenType::LBrace);
		while (!CheckToken(TokenType::RBrace))
		{
			ConsumeToken({ TokenType::Identifier, TokenType::Pass });
			{
				//Comp Always;
				//Pass Keep;
				//Fail Keep;
				//ZFail Keep;
				auto& token = PreviousToken().text;
				if (token == "Ref" || token == "ReadMask" || token == "WriteMask")
				{
					ConsumeToken(TokenType::Number);
					if (token == "Ref")
						stencilNode.state.ref = std::stoi(PreviousToken().text);
					else if (token == "ReadMask")
						stencilNode.state.compareMask = std::stoi(PreviousToken().text);
					else if (token == "WriteMask")
						stencilNode.state.writeMask = std::stoi(PreviousToken().text);
				}
				else if (token == "Comp")
				{
					ConsumeToken(TokenType::Identifier);
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
						stencilNode.state.compareOp = static_cast<StencilState::CompareOp>(idx);
					}
				}
				else if (token == "Pass" || token == "Fail" || token == "ZFail")
				{
					ConsumeToken(TokenType::Identifier);
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
							stencilNode.state.passOp = static_cast<StencilState::StencilOp>(idx);
						else if (token == "Fail")
							stencilNode.state.failOp = static_cast<StencilState::StencilOp>(idx);
						else if (token == "ZFail")
							stencilNode.state.depthFailOp = static_cast<StencilState::StencilOp>(idx);
					}
				}
				else
				{
					std::string err = "Allowed identifiers: Ref, ReadMask, WriteMask, Comp, Pass, Fail, ZFail";
					throw ShaderParserException{ GetTokenErrorString(err).c_str() };
				}
				ConsumeToken(TokenType::Semicolon);
			}
		}
		ConsumeToken(TokenType::RBrace);
		passNode.stencil = stencilNode;
	}
	auto ShaderParser::ParseStage() -> ShaderAST::StageNode
	{
		ShaderAST::StageNode stage;
		ConsumeToken(TokenType::Stage);

		if (CheckToken(TokenType::Vertex))
		{
			stage.type = ShaderAST::StageType::Vertex;
			NextToken();
		}
		else if (CheckToken(TokenType::Fragment))
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

		ConsumeToken(TokenType::LBrace);

		stage.code = GetStageBody();
		ParseStageBody(stage);

		ConsumeToken(TokenType::RBrace);

		return stage;
	}
	auto ShaderParser::GetStageBody() -> std::string
	{
		std::string code{};
		int curPos = pos;
		// 이미 '{'는 읽은 뒤라 nesting = 1 로 시작
		int nesting = 1;
		while (!CheckToken(TokenType::EndOfFile))
		{
			if (PeekToken().type == TokenType::Literal)
				code.pop_back();

			code += PeekToken().text;
			code += ' ';
			if (CheckToken(TokenType::LBrace))
			{
				NextToken();
				nesting++;
			}
			else if (CheckToken(TokenType::RBrace))
			{
				nesting--;
				if (nesting == 0) // Stage 끝
				{
					code.erase(code.size() - 2); // 마지막에 들어간 } 지우기
					break;
				}
				else
					NextToken();
			}
			else
				NextToken();
		}
		pos = curPos;
		return code;
	}
	void ShaderParser::ParseStageBody(ShaderAST::StageNode& stageNode)
	{
		// 이미 '{'는 읽은 뒤라 nesting = 1 로 시작
		int nesting = 1;
		while (!CheckToken(TokenType::EndOfFile))
		{
			if (CheckToken(TokenType::LBrace))
			{
				NextToken();
				nesting++;
			}
			else if (CheckToken(TokenType::RBrace))
			{
				nesting--;
				if (nesting == 0) // Stage 끝
					break;
				else
					NextToken();
			}
			else if (CheckToken(TokenType::Layout))
				ParseLayout(stageNode);
			else
				NextToken();
		}
	}
	void ShaderParser::ParseLayout(ShaderAST::StageNode& stageNode)
	{
		NextToken();
		ShaderAST::LayoutNode layoutNode{};
		ShaderAST::UBONode uboNode{};
		ConsumeToken(TokenType::LBracket); // (

		ConsumeToken(TokenType::Identifier);

		bool bUniform = false;
		if (PreviousToken().text == "location") // 어트리뷰트
		{
			ConsumeToken(TokenType::Operator);
			ConsumeToken(TokenType::Number);
			layoutNode.binding = std::stoi(PreviousToken().text);
		}
		else if (PreviousToken().text == "set") // 유니폼
		{
			bUniform = true;
			ConsumeToken(TokenType::Operator);
			ConsumeToken(TokenType::Number);
			uboNode.set = std::stoi(PreviousToken().text);
			ConsumeToken(TokenType::Unknown); // ','
			ConsumeToken(TokenType::Identifier);
			if (PreviousToken().text == "binding")
			{
				ConsumeToken(TokenType::Operator);
				ConsumeToken(TokenType::Number);
				uboNode.binding = std::stoi(PreviousToken().text);
			}
			else
				throw ShaderParserException{ GetTokenErrorString("Not found binding token").c_str() };
		}
		ConsumeToken(TokenType::RBracket); // )

		if (!bUniform) // 어트리뷰트 in, out 처리
		{
			auto& inoutToken = PeekToken();
			if (CheckToken(TokenType::In) || CheckToken(TokenType::Out))
			{
				NextToken();
				ShaderAST::VariableNode varNode{};
				ConsumeToken(TokenType::Identifier);
				varNode.type = IdentifierToVaraibleType(PreviousToken());
				ConsumeToken(TokenType::Identifier);
				varNode.name = PreviousToken().text;

				layoutNode.var = std::move(varNode);
				if (inoutToken.type == TokenType::In)
					stageNode.in.push_back(layoutNode);
				else
					stageNode.out.push_back(layoutNode);
			}
			else
				throw ShaderParserException{ GetTokenErrorString("Not found 'in' or 'out' keyword").c_str() };
		}
		else // Uniform
		{
			ConsumeToken(TokenType::Uniform);
			if (CheckToken(TokenType::Identifier))
			{
				NextToken();
				ParseUniformBody(uboNode);
				ConsumeToken(TokenType::Identifier);
				uboNode.name = PreviousToken().text;
				ConsumeToken(TokenType::Semicolon);
			}
			else if (CheckToken(TokenType::Sampler2D))
			{
				uboNode.bSampler = true;
				NextToken();
				ConsumeToken(TokenType::Identifier);
				uboNode.name = PreviousToken().text;
				ConsumeToken(TokenType::Semicolon);
			}
			else
				throw ShaderParserException({ GetTokenErrorString("Not found Identifier or Sampler2D keyword").c_str()});

			stageNode.uniforms.push_back(std::move(uboNode));
		}
	}
	void ShaderParser::ParseUniformBody(ShaderAST::UBONode& uboNode)
	{
		ConsumeToken(TokenType::LBrace); // {

		while (!CheckToken(TokenType::RBrace))
		{
			ShaderAST::VariableNode varNode{};
			ConsumeToken(TokenType::Identifier);
			varNode.type = IdentifierToVaraibleType(PreviousToken());
			ConsumeToken(TokenType::Identifier);
			varNode.name = PreviousToken().text;
			if (CheckToken(TokenType::LSquareBracket)) // 배열
			{
				NextToken();
				ConsumeToken(TokenType::Number);
				varNode.size = std::stoi(PreviousToken().text);
				ConsumeToken(TokenType::RSquareBracket);
			}
			ConsumeToken(TokenType::Semicolon);

			uboNode.vars.push_back(std::move(varNode));
		}

		ConsumeToken(TokenType::RBrace); // }
	}
	void ShaderParser::Lex(const std::string& source)
	{
		int line = 1;
		int column = 1;
		const std::size_t length = source.size();
		std::size_t i = 0;

		auto AddToken = [&](TokenType t, const std::string& txt)
			{
				tokens.push_back({ t, txt, line, column });
			};

		while (i < length)
		{
			char c = source[i];
			if (c == ' ' || c == '\t' || c == '\r')
			{
				++i;
				++column;
				continue;
			}
			if (c == '\n')
			{
				++i;
				++line;
				column = 0;
				continue;
			}

			if (c == '{')
			{
				AddToken(TokenType::LBrace, "{");
				++i;
				++column;
				continue;
			}
			if (c == '}')
			{
				AddToken(TokenType::RBrace, "}");
				++i;
				++column;
				continue;
			}
			if (c == '(')
			{
				AddToken(TokenType::LBracket, "(");
				++i;
				++column;
				continue;
			}
			if (c == ')')
			{
				AddToken(TokenType::RBracket, ")");
				++i;
				++column;
				continue;
			}
			if (c == '[')
			{
				AddToken(TokenType::LSquareBracket, "[");
				++i;
				++column;
				continue;
			}
			if (c == ']')
			{
				AddToken(TokenType::RSquareBracket, "]");
				++i;
				++column;
				continue;
			}
			if (c == '=')
			{
				++i;
				++column;
				if (i + 1 < source.size())
				{
					if (source[i] == '=')
					{
						AddToken(TokenType::Operator, "==");
						++i;
						++column;
						continue;
					}
				}
				AddToken(TokenType::Operator, "=");
				continue;
			}
			if (c == '+')
			{
				++i;
				++column;
				if (i < source.size())
				{
					if (source[i] == '+' || source[i] == '=')
					{
						AddToken(TokenType::Operator, std::string{ c } + source[i]);
						++i;
						++column;
						continue;
					}
				}
				AddToken(TokenType::Operator, std::string{ c });
				continue;
			}
			if (c == '-')
			{
				++i;
				++column;
				if (i < source.size())
				{
					if (source[i] == '-' || source[i] == '=')
					{
						AddToken(TokenType::Operator, std::string{ c } + source[i]);
						++i;
						++column;
						continue;
					}
				}
				AddToken(TokenType::Operator, std::string{ c });
				continue;
			}
			if (c == '*')
			{
				++i;
				++column;
				if (i < source.size())
				{
					if (source[i] == '=')
					{
						AddToken(TokenType::Operator, std::string{ c } + source[i]);
						++i;
						++column;
						continue;
					}
				}
				AddToken(TokenType::Operator, std::string{ c });
				continue;
			}
			if (c == '/')
			{
				++i;
				++column;
				if (i < source.size())
				{
					if (source[i] == '=')
					{
						AddToken(TokenType::Operator, std::string{ c } + source[i]);
						++i;
						++column;
						continue;
					}
				}
				AddToken(TokenType::Operator, std::string{ c });
				continue;
			}

			if (c == ';')
			{
				AddToken(TokenType::Semicolon, ";");
				++i;
				++column;
				continue;
			}

			if (c == '\"')
			{
				++i;
				++column;

				std::string strValue;
				bool closed = false;

				while (i < length)
				{
					char c2 = source[i];
					if (c2 == '\"')
					{
						closed = true;
						++i;
						++column;
						break;
					}
					strValue.push_back(c2);
					++i;
					++column;
				}
				if (!closed)
					throw ShaderParserException{ fmt::format("}} not found! (line: {})", line).c_str() };

				AddToken(TokenType::String, strValue);
				continue;
			}
			// 숫자
			if (std::isdigit(c))
			{
				std::string number;
				while (i < length && (std::isdigit(source[i]) || source[i] == '.'))
				{
					number.push_back(source[i]);
					++i;
					++column;
				}
				AddToken(TokenType::Number, number);
				if (source[i] == 'f')
				{
					++i;
					++column;
					AddToken(TokenType::Literal, "f");
				}
				continue;
			}
			// 식별자 또는 키워드
			if (std::isalpha(c))
			{
				std::string ident;
				while (i < length && IsIdentifierChar(source[i]))
				{
					ident.push_back(source[i]);
					++i;
					++column;
				}

				if (ident == "Shader")
					AddToken(TokenType::Shader, ident);
				else if (ident == "Pass")
					AddToken(TokenType::Pass, ident);
				else if (ident == "Stage")
					AddToken(TokenType::Stage, ident);
				else if (ident == "Vertex")
					AddToken(TokenType::Vertex, ident);
				else if (ident == "Fragment")
					AddToken(TokenType::Fragment, ident);
				else if (ident == "layout")
					AddToken(TokenType::Layout, ident);
				else if (ident == "in")
					AddToken(TokenType::In, ident);
				else if (ident == "out")
					AddToken(TokenType::Out, ident);
				else if (ident == "uniform")
					AddToken(TokenType::Uniform, ident);
				else if (ident == "sampler2D")
					AddToken(TokenType::Sampler2D, ident);
				else if (ident == "Stencil")
					AddToken(TokenType::Stencil, ident);
				else
					AddToken(TokenType::Identifier, ident);

				continue;
			}

			if (c == '#')
			{
				AddToken(TokenType::Preprocessor, "#");
				++i;
				++column;
				continue;
			}

			AddToken(TokenType::Unknown, std::string{ c });
			++i;
			++column;
		}

		AddToken(TokenType::EndOfFile, "");
	}
	SH_RENDER_API auto sh::render::ShaderParser::Parse(const std::string& source) -> ShaderAST::ShaderNode
	{
		Lex(source);

		return ParseShader();
	}
}//namespace