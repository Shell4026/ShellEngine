#pragma once
#include "Export.h"
#include "ShaderAST.h"
#include "ComputeShaderLexer.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace sh::render
{
	class ComputeShaderParser
	{
	public:
		SH_RENDER_API auto Parse(const std::vector<ComputeShaderLexer::Token>& tokens) -> ShaderAST::ComputeShaderNode;
	protected:
		/// @brief 현재 토큰을 반환하는 함수.
		auto PeekToken() const -> const ComputeShaderLexer::Token&;
		/// @brief 현재 토큰 타입 검사 함수.
		auto CheckToken(ComputeShaderLexer::TokenType token) const -> bool;
		/// @brief 다음 토큰으로 이동하는 함수.
		auto NextToken() -> const ComputeShaderLexer::Token&;
		/// @brief 직전에 소모된 토큰을 반환하는 함수.
		auto PreviousToken() -> const ComputeShaderLexer::Token&;
		/// @brief 해당 토큰이 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		void ConsumeToken(ComputeShaderLexer::TokenType token);
		/// @brief 해당 토큰들 중 하나라도 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		void ConsumeToken(const std::initializer_list<ComputeShaderLexer::TokenType>& tokens);

		auto GetCurrentTokenPosString() const -> std::string;
		auto GetTokenErrorString(const std::string& msg) const -> std::string;

		auto ParseComputeShader() -> ShaderAST::ComputeShaderNode;
		void ParsePreprocessor(ShaderAST::ComputeShaderNode& node);
		void ParseNumthreads(ShaderAST::ComputeShaderNode& node);
		void ParseBuffer(ShaderAST::ComputeShaderNode& node, ShaderAST::BufferAccess access);
		void ParseDeclaration(ShaderAST::ComputeShaderNode& node, const std::string& qualifier = "");
		auto ParseFunctionBody() -> std::string;

		static auto IdentifierToVariableType(const ComputeShaderLexer::Token& token) -> ShaderAST::VariableType;
		static auto VariableTypeToString(ShaderAST::VariableType type) -> std::string;
		static void GenerateCode(ShaderAST::ComputeShaderNode& node);
	private:
		const std::vector<ComputeShaderLexer::Token>* tokens = nullptr;
		std::size_t pos = 0;
		uint32_t lastBinding = 0;
	};

	class ComputeShaderParserException : public std::runtime_error
	{
	public:
		ComputeShaderParserException(const char* err) : std::runtime_error{ err }
		{}
	};
}//namespace
