#pragma once
#include "Export.h"

#include <vector>
#include <stdexcept>
namespace sh::render
{
	class ShaderLexer
	{
	public:
		enum class TokenType
		{
			Preprocessor,
			Shader, Pass, Stage, LightingPass, Property,
			Stencil,
			Vertex, Fragment,
			Layout, Uniform, In, Out, Sampler2D,
			Const,
			VERTEX, UV, NORMAL, MVP, LIGHT,
			LBracket, // (
			RBracket, // )
			LBrace, // {
			RBrace, // }
			LSquareBracket, // [
			RSquareBracket, // ]
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
	private:
		/// @brief 알파벳/숫자/언더스코어로 구성된 식별자인지 검사하는 헬퍼 함수.
		/// @param c 문자
		static auto IsIdentifierChar(char c) -> bool;
	public:
		SH_RENDER_API static auto Lex(const std::string& source) -> std::vector<Token>;
	};

	class ShaderLexerException : public std::exception
	{
	private:
		std::string err;
	public:
		ShaderLexerException(const std::string& err) : err(err) {}
		auto what() const noexcept -> const char* override { return err.c_str(); }
	};
}
