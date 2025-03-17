#pragma once
#include "Export.h"
#include "ShaderAST.hpp"
#include "ShaderLexer.h"

#include "Core/ArrayView.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <initializer_list>

namespace sh::render
{
	class ShaderParser
	{
	private:
		const std::vector<ShaderLexer::Token>* tokens;

		std::size_t pos; // 현재 토큰 위치

		uint32_t lastObjectUniformBinding = 0;
		uint32_t lastMaterialUniformBinding = 0;
		uint32_t passCount = 1;
	protected:
		/// @brief 현재 토큰을 반환하는 함수.
		/// @return 토큰
		auto PeekToken() const -> const ShaderLexer::Token&;
		/// @brief 현재 토큰 타입 검사 함수.
		/// @return 토큰이 일치하면 true
		auto CheckToken(ShaderLexer::TokenType token) const -> bool;
		/// @brief 다음 토큰으로 이동하는 함수.
		/// @return 현재 토큰
		auto NextToken() -> const ShaderLexer::Token&;
		/// @brief 직전에 소모된 토큰을 반환 하는 함수.
		/// @return 직전에 소모된 토큰
		auto PreviousToken() -> const ShaderLexer::Token&;
		/// @brief 해당 토큰이 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		/// @param token 토큰
		void ConsumeToken(ShaderLexer::TokenType token);
		/// @brief 해당 토큰들 중 하나라도 존재하면 다음 토큰으로 이동하며 없다면 예외를 던진다.
		/// @param tokens 토큰 목록
		void ConsumeToken(const std::initializer_list<ShaderLexer::TokenType>& tokens);

		auto IdentifierToVaraibleType(const ShaderLexer::Token& token) const -> ShaderAST::VariableType;
		auto VariableTypeToString(ShaderAST::VariableType type) const -> std::string;

		/// @brief (line: {}, col: {}) 형식의 현재 토큰 위치의 문자열을 반환한다.
		auto GetCurrentTokenPosString() const -> std::string;
		auto GetNotFoundTokenString(const std::initializer_list<ShaderLexer::TokenType>& tokens) -> std::string;
		auto GetTokenErrorString(const std::string& msg) const -> std::string;

		auto ParseShader() -> ShaderAST::ShaderNode;
		void ParsePreprocessor(ShaderAST::ShaderNode& shaderNode);
		void ParseProperty(ShaderAST::ShaderNode& shaderNode);
		auto ParsePass(const ShaderAST::ShaderNode& shaderNode) -> ShaderAST::PassNode;
		void ParseLightingPass(ShaderAST::PassNode& passNode);
		void ParseStencil(ShaderAST::PassNode& passNode);
		void ParseCull(ShaderAST::PassNode& passNode);
		void ParseZWrite(ShaderAST::PassNode& passNode);
		void ParseColorMask(ShaderAST::PassNode& passNode);
		auto ParseStage(const ShaderAST::ShaderNode& shaderNode) -> ShaderAST::StageNode;
		void ParseStageBody(const ShaderAST::ShaderNode& shaderNode, ShaderAST::StageNode& stageNode);
		void ParseDeclaration(ShaderAST::StageNode& stageNode, const std::string& qualifer = "");
		auto ParseFunctionBody(ShaderAST::StageNode& stageNode) -> std::string;
		void ParseLayout(ShaderAST::StageNode& stageNode);
		void ParseUniform(const ShaderAST::ShaderNode& shaderNode, ShaderAST::StageNode& stageNode);
		void ParseUniformBody(ShaderAST::UBONode& uboNode);

		void Optimize(ShaderAST::ShaderNode& shaderNode);
		void GenerateStageCode(int stageIdx, const ShaderAST::ShaderNode& shaderNode, ShaderAST::StageNode& stageNode);
		auto SubstitutionFunctionToken(const ShaderLexer::Token& token) const -> std::string;
	public:
		SH_RENDER_API auto Parse(const std::vector<ShaderLexer::Token>& tokens) -> ShaderAST::ShaderNode;
	};

	class ShaderParserException : public std::runtime_error
	{
	public:
		ShaderParserException(const char* err) : std::runtime_error{ err }
		{}
	};
}//namespace