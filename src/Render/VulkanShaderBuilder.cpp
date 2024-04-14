#include "VulkanShaderBuilder.h"

#include "VulkanShader.h"

#include <cassert>

namespace sh::render
{
	VulkanShaderBuilder::VulkanShaderBuilder(VkDevice device) :
		device(device)
	{
	}

	VulkanShaderBuilder::~VulkanShaderBuilder()
	{
	}

	auto VulkanShaderBuilder::Build() -> std::unique_ptr<Shader>
	{
		
		VkShaderModule vertShader{nullptr}, fragShader{nullptr};

		assert(vertShaderData.data());
		assert(fragShaderData.data());
		//Vertex shader
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = vertShaderData.size();
		info.pCode = reinterpret_cast<const uint32_t*>(vertShaderData.data());

		VkResult result = vkCreateShaderModule(device, &info, nullptr, &vertShader);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return nullptr;

		//Fragment shader
		info.codeSize = fragShaderData.size();
		info.pCode = reinterpret_cast<const uint32_t*>(fragShaderData.data());

		result = vkCreateShaderModule(device, &info, nullptr, &fragShader);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return nullptr;

		std::unique_ptr<Shader> retShader = std::make_unique<VulkanShader>(device);
		static_cast<VulkanShader*>(retShader.get())->SetVertexShader(vertShader);
		static_cast<VulkanShader*>(retShader.get())->SetFragmentShader(fragShader);

		return retShader;
	}
}