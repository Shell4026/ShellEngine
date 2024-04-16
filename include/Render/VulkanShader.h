#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"
#include "Shader.h"

#include <vector>

namespace sh::render
{
	class VulkanShader : public Shader
	{
		SCLASS(VulkanShader)
	private:
		VkShaderModule vertShader;
		VkShaderModule fragShader;

		VkDevice device;
	public:
		SH_RENDER_API VulkanShader(VkDevice device);
		SH_RENDER_API ~VulkanShader();

		SH_RENDER_API void SetVertexShader(VkShaderModule shader);
		SH_RENDER_API void SetFragmentShader(VkShaderModule shader);

		SH_RENDER_API auto GetVertexShader() const -> const VkShaderModule;
		SH_RENDER_API auto GetFragmentShader() const -> const VkShaderModule;

		SH_RENDER_API void Clean() override;
	};
}