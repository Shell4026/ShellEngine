#include "Shader.h"

namespace sh::render
{
	SH_RENDER_API void Shader::AddPass(std::unique_ptr<ShaderPass>&& pass)
	{
		passes.push_back(std::move(pass));
	}

	SH_RENDER_API auto Shader::GetPass(std::size_t idx) const -> ShaderPass*
	{
		return passes[idx].get();
	}

	SH_RENDER_API auto Shader::GetPasses() const -> const std::vector<std::unique_ptr<ShaderPass>>&
	{
		return passes;
	}
}//namepsace
