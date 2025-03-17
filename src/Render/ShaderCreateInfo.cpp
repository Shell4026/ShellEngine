#include "ShaderCreateInfo.h"
#include "ShaderPass.h"

namespace sh::render
{
	ShaderCreateInfo::ShaderCreateInfo()
	{
	}
	SH_RENDER_API void ShaderCreateInfo::Clear()
	{
		passes.clear();
	}
	SH_RENDER_API auto ShaderCreateInfo::SetShaderNode(const ShaderAST::ShaderNode& shaderNode) -> ShaderCreateInfo&
	{
		this->shaderNode = shaderNode;
		return *this;
	}
	SH_RENDER_API auto sh::render::ShaderCreateInfo::AddShaderPass(std::unique_ptr<ShaderPass>&& shaderPass) -> ShaderCreateInfo&
	{
		passes.push_back(std::move(shaderPass));
		return *this;
	}
	SH_RENDER_API auto ShaderCreateInfo::GetShaderNode() const -> const ShaderAST::ShaderNode&
	{
		return shaderNode;
	}
	SH_RENDER_API auto ShaderCreateInfo::GetShaderPasses() const -> const std::vector<std::unique_ptr<ShaderPass>>&
	{
		return passes;
	}
	SH_RENDER_API auto ShaderCreateInfo::GetShaderPasses() -> std::vector<std::unique_ptr<ShaderPass>>&
	{
		return passes;
	}
}//namespace