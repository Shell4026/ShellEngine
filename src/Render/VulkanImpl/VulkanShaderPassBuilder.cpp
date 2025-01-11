#include "VulkanShaderPassBuilder.h"
#include "VulkanContext.h"
#include "VulkanShaderPass.h"

#include <cassert>

namespace sh::render::vk
{
	VulkanShaderPassBuilder::VulkanShaderPassBuilder(const render::vk::VulkanContext& context) :
		context(context)
	{
	}

	VulkanShaderPassBuilder::~VulkanShaderPassBuilder()
	{
	}

	SH_RENDER_API auto VulkanShaderPassBuilder::Build() -> std::unique_ptr<render::ShaderPass>
	{
		assert(context.GetDevice() != nullptr);
		VkShaderModule vertShader{ nullptr }, fragShader{ nullptr };

		assert(vertShaderData.data());
		assert(fragShaderData.data());

		// Vertex shader
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = vertShaderData.size();
			info.pCode = reinterpret_cast<const uint32_t*>(vertShaderData.data());

			VkResult result = vkCreateShaderModule(context.GetDevice(), &info, nullptr, &vertShader);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return nullptr;
		}

		// Fragment shader
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = fragShaderData.size();
			info.pCode = reinterpret_cast<const uint32_t*>(fragShaderData.data());

			VkResult result = vkCreateShaderModule(context.GetDevice(), &info, nullptr, &fragShader);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return nullptr;
		}


		auto retShader = std::make_unique<render::vk::VulkanShaderPass>(GetNextId(), context.GetDevice());
		retShader->SetVertexShader(vertShader);
		retShader->SetFragmentShader(fragShader);

		return retShader;
	}
}