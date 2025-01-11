#pragma once
#include "Export.h"
#include "ShaderAST.hpp"
#include "StencilState.h"

#include <filesystem>
#include <string>
#include <vector>

namespace sh::render
{
	class ShaderGenerator
	{
	private:
		/// @brief 문자열에서 공백을 제거하고 _로 대체하는 함수
		/// @param str 문자열
		/// @return 공백이 대체된 문자열
		static auto ReplaceSpaceString(const std::string& str) -> std::string;
	public:
		/// @brief 경로에 셰이더 파일을 생성하는 함수.
		/// @param shaderName 셰이더 이름
		/// @param versionNode 셰이더 AST트리의 버전 노드
		/// @param shaderNode 셰이더 AST트리의 패스 노드
		/// @param path 디렉토리 경로
		/// @return 생성된 파일들의 경로
		SH_RENDER_API static auto GenerateShaderFile(
			const std::string& shaderName,
			const ShaderAST::VersionNode& versionNode, 
			const ShaderAST::PassNode& passNode, 
			const std::filesystem::path& path) -> std::vector<std::filesystem::path>;
	};
}//namespace