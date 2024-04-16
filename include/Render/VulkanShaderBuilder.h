#pragma once

#include "ShaderBuilder.h"

#include "Export.h"
#include "VulkanImpl/VulkanConfig.h"

#include <vector>

namespace sh::render
{
	class VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		VkDevice device;
	public:
		SH_RENDER_API VulkanShaderBuilder(VkDevice device);
		SH_RENDER_API ~VulkanShaderBuilder();

		SH_RENDER_API auto Build()->std::unique_ptr<Shader> override;
	};
}