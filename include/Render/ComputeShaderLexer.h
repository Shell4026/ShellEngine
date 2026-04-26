#pragma once
#include "Export.h"

#include <string>
#include <vector>
#include <stdexcept>
namespace sh::render
{
	class ComputeShaderLexer
	{
	public:
		enum class TokenType
		{
			Preprocessor,
			Numthreads,
			RBuffer, WBuffer, RWBuffer,
			Const,
			LBracket,        // (
			RBracket,        // )
			LBrace,          // {
			RBrace,          // }
			LSquareBracket,  // [
			RSquareBracket,  // ]
			Comma,           // ,
			String, Identifier, Literal, Number, Operator, Semicolon,
			EndOfFile,
			Unknown
		};
		struct Token
		{
			TokenType type;
			std::string text;
			int line;
			int column;
		};
	public:
		SH_RENDER_API static auto Lex(const std::string& source) -> std::vector<Token>;
	private:
		static auto IsIdentifierChar(char c) -> bool;
	};

	class ComputeShaderLexerException : public std::exception
	{
	public:
		ComputeShaderLexerException(const std::string& err) : err(err) {}

		auto what() const noexcept -> const char* override { return err.c_str(); }
	private:
		std::string err;
	};
}//namespace
