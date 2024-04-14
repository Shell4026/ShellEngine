#pragma once

#include "ShaderBuilder.h"

#include "Export.h"
#include "VulkanImpl/VulkanConfig.h"

#include <vector>

namespace sh::render
{
	class SH_RENDER_API VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		VkDevice device;
	public:
		VulkanShaderBuilder(VkDevice device);
		~VulkanShaderBuilder();

		auto Build()->std::unique_ptr<Shader> override;
	};
}