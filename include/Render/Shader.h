#pragma once
#include "Export.h"
#include "ShaderPass.h"

#include "Core/SObject.h"

#include <vector>
#include <memory>
namespace sh::render
{
	class Shader : public core::SObject
	{
		SCLASS(Shader)
	private:
		std::vector<std::unique_ptr<ShaderPass>> passes;
	public:
		SH_RENDER_API void AddPass(std::unique_ptr<ShaderPass>&& pass);
		SH_RENDER_API auto GetPass(std::size_t idx) const -> ShaderPass*;
		SH_RENDER_API auto GetPasses() const -> const std::vector<std::unique_ptr<ShaderPass>>&;
	};
}//namespace
