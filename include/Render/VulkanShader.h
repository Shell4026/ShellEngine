#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"
#include "Shader.h"

#include <vector>

namespace sh::render
{
	class SH_RENDER_API VulkanShader : public Shader
	{
		SCLASS(VulkanShader)
	private:
		VkShaderModule vertShader;
		VkShaderModule fragShader;

		VkDevice device;
	public:
		VulkanShader(VkDevice device);
		~VulkanShader();

		void SetVertexShader(VkShaderModule shader);
		void SetFragmentShader(VkShaderModule shader);

		auto GetVertexShader() const -> const VkShaderModule;
		auto GetFragmentShader() const -> const VkShaderModule;

		void Clean() override;
	};
}