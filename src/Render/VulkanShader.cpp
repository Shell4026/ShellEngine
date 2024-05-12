#include "VulkanShader.h"

#include <cassert>

namespace sh::render
{
	VulkanShader::VulkanShader(int id, VkDevice device) :
		Shader(id, ShaderType::SPIR),
		device(device), vertShader(nullptr), fragShader(nullptr)
	{
	}

	VulkanShader::VulkanShader(VulkanShader&& other) noexcept :
		Shader(std::move(other)),
		vertShader(other.vertShader), fragShader(other.fragShader),
		device(device)
	{
		other.vertShader = nullptr;
		other.fragShader = nullptr;
		other.device = nullptr;
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