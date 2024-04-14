#include "VulkanShader.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanShader::VulkanShader(VkDevice device) :
		Shader(ShaderType::SPIR),
		vertShader(nullptr), fragShader(nullptr),
		device(device)
	{
	}

	void VulkanShader::SetVertexShader(VkShaderModule shader)
	{
		vertShader = shader;
	}

	void VulkanShader::SetFragmentShader(VkShaderModule shader)
	{
		fragShader = shader;
	}

	void VulkanShader::Clean()
	{
		assert(device);
		if (vertShader)
		{
			vkDestroyShaderModule(device, vertShader, nullptr);
			vertShader = nullptr;
		}
		if (fragShader)
		{
			vkDestroyShaderModule(device, fragShader, nullptr);
			fragShader = nullptr;
		}
	}
}