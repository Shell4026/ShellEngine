#pragma once

#include "ShaderBuilder.h"

#include "Export.h"

#include <vector>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		VulkanRenderer& renderer;
	public:
		SH_RENDER_API VulkanShaderBuilder(VulkanRenderer& renderer);
		SH_RENDER_API ~VulkanShaderBuilder();

		SH_RENDER_API auto Build()->std::unique_ptr<Shader> override;
	};
}