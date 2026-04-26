#include "ComputeShaderLexer.h"

#include <fmt/core.h>

#include <cctype>
#include <unordered_map>

namespace sh::render
{
	SH_RENDER_API auto ComputeShaderLexer::Lex(const std::string& source) -> std::vector<Token>
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
					if (source[i + 1] == '/') // //
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
			if (c == ',')
			{
				AddToken(TokenType::Comma, ",");
				++i;
				++column;
				continue;
			}
			// 두 글자 합성 연산자가 가능한 연산자들
			auto LexCompoundOp = [&](char single, std::initializer_list<char> followers)
				{
					std::string op{ single };
					++i;
					++column;
					if (i < source.size())
					{
						for (char f : followers)
						{
							if (source[i] == f)
							{
								op.push_back(source[i]);
								++i;
								++column;
								break;
							}
						}
					}
					AddToken(TokenType::Operator, op);
				};
			if (c == '=') { LexCompoundOp('=', { '=' }); continue; }
			if (c == '+') { LexCompoundOp('+', { '+', '=' }); continue; }
			if (c == '-') { LexCompoundOp('-', { '-', '=' }); continue; }
			if (c == '*') { LexCompoundOp('*', { '=' }); continue; }
			if (c == '/') { LexCompoundOp('/', { '=' }); continue; }
			if (c == '<') { LexCompoundOp('<', { '=', '<' }); continue; }
			if (c == '>') { LexCompoundOp('>', { '=', '>' }); continue; }
			if (c == '!') { LexCompoundOp('!', { '=' }); continue; }
			if (c == '&') { LexCompoundOp('&', { '&', '=' }); continue; }
			if (c == '|') { LexCompoundOp('|', { '|', '=' }); continue; }
			if (c == '%') { LexCompoundOp('%', { '=' }); continue; }
			if (c == '^') { LexCompoundOp('^', { '=' }); continue; }

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
					throw ComputeShaderLexerException{ fmt::format("\" not closed! (line: {})", line) };
				AddToken(TokenType::String, strValue);
				continue;
			}
			// 숫자
			if (std::isdigit(static_cast<unsigned char>(c)))
			{
				std::string number;
				while (i < length && (std::isdigit(static_cast<unsigned char>(source[i])) || source[i] == '.'))
				{
					number.push_back(source[i]);
					++i;
					++column;
				}
				AddToken(TokenType::Number, number);
				if (i < length && source[i] == 'f')
				{
					++i;
					++column;
					AddToken(TokenType::Literal, "f");
				}
				continue;
			}
			// 식별자 또는 키워드
			if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
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
					{"Numthreads", TokenType::Numthreads},
					{"RBuffer", TokenType::RBuffer},
					{"WBuffer", TokenType::WBuffer},
					{"RWBuffer", TokenType::RWBuffer},
					{"const", TokenType::Const}
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

	auto ComputeShaderLexer::IsIdentifierChar(char c) -> bool
	{
		return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
	}
}//namespace
