#include "VulkanShader.h"

#include <cassert>

namespace sh::render
{
	VulkanShader::VulkanShader(VkDevice device) :
		Shader(ShaderType::SPIR),
		vertShader(nullptr), fragShader(nullptr),
		device(device)
	{
	}
	VulkanShader::~VulkanShader()
	{
		Clean();
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

	auto VulkanShader::GetVertexShader() const -> const VkShaderModule
	{
		return vertShader;
	}

	auto VulkanShader::GetFragmentShader() const -> const VkShaderModule
	{
		return fragShader;
	}
}