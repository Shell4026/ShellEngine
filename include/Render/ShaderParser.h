#pragma once
#include "Export.h"
#include "ShaderAST.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace sh::render
{
	class ShaderParser
	{
	private:
		enum class TokenType
		{
			Preprocessor,
			Shader, Pass, Stage,
			Stencil,
			Vertex, Fragment,
			Layout, Uniform, In, Out, Sampler2D,
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

		std::vector<Token> tokens;

		std::size_t pos; // 현재 토큰 위치
	private:
		/// @brief 어휘 분석 함수.
		/// @param source 
		void Lex(const std::string& source);
		/// @brief 알파벳/숫자/언더스코어로 구성된 식별자인지 검사하는 헬퍼 함수.
		/// @param c 문자
		auto IsIdentifierChar(char c) -> bool;
		/// @brief 현재 토큰을 반환하는 함수.
		/// @return 토큰
		auto PeekToken() const -> const Token&;
		/// @brief 현재 토큰 타입 검사 함수.
		/// @return 토큰이 일치하면 true
		auto CheckToken(TokenType token) const -> bool;
		/// @brief 다음 토큰으로 이동하는 함수.
		/// @return 현재 토큰
		auto NextToken() -> const Token&;
		/// @brief 직전에 소모된 토큰을 반환 하는 함수.
		/// @return 직전에 소모된 토큰
		auto PreviousToken() -> const Token&;
		/// @brief 해당 토큰이 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		/// @param token 토큰
		void ConsumeToken(TokenType token);
		/// @brief 해당 토큰들 중 하나라도 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		/// @param tokens 토큰 목록
		void ConsumeToken(const std::initializer_list<TokenType>& tokens);

		auto IdentifierToVaraibleType(const Token& token) const -> ShaderAST::VariableType;

		auto GetNotFoundTokenString(const std::initializer_list<TokenType>& tokens) -> std::string;
		auto GetTokenErrorString(const std::string& msg) const -> std::string;

		auto ParseShader() -> ShaderAST::ShaderNode;
		void ParsePreprocessor(ShaderAST::ShaderNode& shaderNode);
		auto ParsePass() -> ShaderAST::PassNode;
		void ParseStencil(ShaderAST::PassNode& passNode);
		auto ParseStage() -> ShaderAST::StageNode;
		auto GetStageBody() -> std::string;
		void ParseStageBody(ShaderAST::StageNode& stageNode);
		void ParseLayout(ShaderAST::StageNode& stageNode);
		void ParseUniformBody(ShaderAST::UBONode& uboNode);
	public:
		SH_RENDER_API auto Parse(const std::string& source) -> ShaderAST::ShaderNode;
	};

	class ShaderParserException : public std::runtime_error
	{
	public:
		ShaderParserException(const char* err) : std::runtime_error{ err }
		{}
	};
}//namespace