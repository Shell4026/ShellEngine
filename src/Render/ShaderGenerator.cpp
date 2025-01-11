#include "ShaderGenerator.h"

#include "Core/FileSystem.h"

#include <fmt/core.h>

#include <cctype>

namespace sh::render
{
	auto ShaderGenerator::ReplaceSpaceString(const std::string& str) -> std::string
	{
		std::string result;
		result.reserve(str.size());
		for (char c : str)
		{
			if (std::isalnum(static_cast<unsigned char>(c)))
				result.push_back(c);
			else
				result.push_back('_');
		}
		return result;
	}

	SH_RENDER_API auto ShaderGenerator::GenerateShaderFile(
		const std::string& shaderName,
		const ShaderAST::VersionNode& versionNode, 
		const ShaderAST::PassNode& passNode, 
		const std::filesystem::path& path) -> std::vector<std::filesystem::path>
	{
		std::vector<std::filesystem::path> results;
		std::string version{ fmt::format("#version {} {}", versionNode.versionNumber, versionNode.profile) };
		for (auto& stage : passNode.stages)
		{
			std::string fileName{ ReplaceSpaceString(shaderName + "_" + passNode.name) };
			if (stage.type == ShaderAST::StageType::Vertex)
				fileName += ".vert";
			else if (stage.type == ShaderAST::StageType::Fragment)
				fileName += ".frag";

			std::string code = fmt::format("{}\n{}", version, stage.code);

			if (core::FileSystem::SaveText(code, path / fileName))
				results.push_back(path / fileName);
		}
		return results;
	}
}//namespace