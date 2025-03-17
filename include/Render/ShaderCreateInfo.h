#pragma once
#include "Export.h"
#include "ShaderAST.hpp"

#include <vector>
#include <memory>
namespace sh::render
{
	class Shader;
	class ShaderPass;
	class ShaderCreateInfo
	{
		ShaderAST::ShaderNode shaderNode;
		std::vector<std::unique_ptr<ShaderPass>> passes;
	public:
		SH_RENDER_API ShaderCreateInfo();
		SH_RENDER_API void Clear();
		SH_RENDER_API auto SetShaderNode(const ShaderAST::ShaderNode& shaderNode) -> ShaderCreateInfo&;
		SH_RENDER_API auto GetShaderNode() const-> const ShaderAST::ShaderNode&;
		SH_RENDER_API auto AddShaderPass(std::unique_ptr<ShaderPass>&& shaderPass) -> ShaderCreateInfo&;
		SH_RENDER_API auto GetShaderPasses() const -> const std::vector<std::unique_ptr<ShaderPass>>&;
		SH_RENDER_API auto GetShaderPasses() -> std::vector<std::unique_ptr<ShaderPass>>&;
	};
}//namespace