#include "ShaderLexer.h"

#include <fmt/core.h>

#include <cctype>
#include <unordered_map>

namespace sh::render
{
	auto ShaderLexer::IsIdentifierChar(char c) -> bool
	{
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}
	SH_RENDER_API auto ShaderLexer::Lex(const std::string& source) -> std::vector<Token>
	{
		std::vector<Token> tokens;

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
			if (c == '\n')
			{
				++i;
				++line;
				column = 0;
				continue;
			}
			if (c == '/') // 주석
			{
				if (i + 1 < length)
				{
					if (source[i + 1] == '/')  // //
					{
						i += 2;
						while (true)
						{
							if (i >= length)
								break;
							if (source[i] == '\n')
							{
								++i;
								column = 0;
								++line;
								break;
							}
							++i;
						}
						continue;
					}
					else if (source[i + 1] == '*') // /**/
					{
						i += 2;
						while (true)
						{
							if (i >= length || i + 1 >= length)
								break;
							if (source[i] == '*' && source[i + 1] == '/')
							{
								i += 2;
								break;
							}
							if (source[i] == '\n')
							{
								column = 0;
								++line;
							}
							if (source[i] == '\r' || source[i] == '\t')
								++column;
							++i;
						}
						continue;
					}
				}
			}
			if (c == ' ' || c == '\t' || c == '\r')
			{
				++i;
				++column;
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
					throw ShaderLexerException{ fmt::format("}} not found! (line: {})", line).c_str() };

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
				static const std::unordered_map<std::string, TokenType> keywordMap = 
				{
					{"Shader", TokenType::Shader},
					{"Pass", TokenType::Pass},
					{"Stage", TokenType::Stage},
					{"Vertex", TokenType::Vertex},
					{"Fragment", TokenType::Fragment},
					{"layout", TokenType::Layout},
					{"in", TokenType::In},
					{"out", TokenType::Out},
					{"uniform", TokenType::Uniform},
					{"sampler2D", TokenType::Sampler2D},
					{"Stencil", TokenType::Stencil},
					{"Cull", TokenType::Cull},
					{"ZWrite", TokenType::Cull},
					{"ColorMask", TokenType::ColorMask},
					{"LightingPass", TokenType::LightingPass},
					{"Property", TokenType::Property},
					{"MVP", TokenType::MVP},
					{"LIGHT", TokenType::LIGHT},
					{"VERTEX", TokenType::VERTEX},
					{"UV", TokenType::UV},
					{"NORMAL", TokenType::NORMAL},
					{"const", TokenType::Const},
					{"MATRIX_MODEL", TokenType::MATRIX_MODEL},
					{"MATRIX_VIEW", TokenType::MATRIX_VIEW},
					{"MATRIX_PROJ", TokenType::MATRIX_PROJ},
				};
				auto it = keywordMap.find(ident);
				if (it == keywordMap.end()) 
					AddToken(TokenType::Identifier, ident);
				else 
					AddToken(it->second, ident);

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
		return tokens;
	}
}//namespace