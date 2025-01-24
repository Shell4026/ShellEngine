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
		const ShaderAST::PassNode& passNode, 
		const std::filesystem::path& path) -> std::vector<std::filesystem::path>
	{
		std::vector<std::filesystem::path> results;
		for (auto& stage : passNode.stages)
		{
			std::string fileName{ ReplaceSpaceString(shaderName + "_" + passNode.name) };
			if (stage.type == ShaderAST::StageType::Vertex)
				fileName += ".vert";
			else if (stage.type == ShaderAST::StageType::Fragment)
				fileName += ".frag";

			if (core::FileSystem::SaveText(stage.code, path / fileName))
				results.push_back(path / fileName);
		}
		return results;
	}
}//namespace