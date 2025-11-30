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
			Stencil, Cull, ZWrite, ColorMask,
			Vertex, Fragment,
			Layout, Uniform, In, Out, Sampler2D, Constexpr,
			Const,
			VERTEX, UV, NORMAL, TANGENT, MVP, LIGHT,
			MATRIX_MODEL, MATRIX_VIEW, MATRIX_PROJ,
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
	public:
		SH_RENDER_API static auto Lex(const std::string& source) -> std::vector<Token>;
	private:
		/// @brief 알파벳/숫자/언더스코어로 구성된 식별자인지 검사하는 헬퍼 함수.
		/// @param c 문자
		static auto IsIdentifierChar(char c) -> bool;
	};

	class ShaderLexerException : public std::exception
	{
	public:
		ShaderLexerException(const std::string& err) : err(err) {}

		auto what() const noexcept -> const char* override { return err.c_str(); }
	private:
		std::string err;
	};
}
