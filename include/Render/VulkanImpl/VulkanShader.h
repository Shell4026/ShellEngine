#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"
#include "Render/Shader.h"

#include <vector>

namespace sh::render::impl
{
	class SH_RENDER_API VulkanShader : public Shader
	{
	private:
		VkShaderModule vertShader;
		VkShaderModule fragShader;

		VkDevice device;
	public:
		VulkanShader(VkDevice device);

		void SetVertexShader(VkShaderModule shader);
		void SetFragmentShader(VkShaderModule shader);

		void Clean() override;
	};
}