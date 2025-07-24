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

	SH_RENDER_API auto VulkanShaderPassBuilder::GetContext() const -> const VulkanContext&
	{
		return context;
	}

	SH_RENDER_API auto VulkanShaderPassBuilder::Build(const ShaderAST::PassNode& passNode) -> render::ShaderPass*
	{
		assert(context.GetDevice() != nullptr);

		assert(vertShaderData.data());
		assert(fragShaderData.data());

		ShaderPass::ShaderCode shaderCode{};
		shaderCode.vert = std::move(vertShaderData);
		shaderCode.frag = std::move(fragShaderData);

		VkShaderModule vertShader = VK_NULL_HANDLE;
		VkShaderModule fragShader = VK_NULL_HANDLE;

		// Vertex shader
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = shaderCode.vert.size();
			info.pCode = reinterpret_cast<const uint32_t*>(shaderCode.vert.data());

			VkResult result = vkCreateShaderModule(context.GetDevice(), &info, nullptr, &vertShader);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return nullptr;
		}

		// Fragment shader
		{
			VkShaderModuleCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.codeSize = shaderCode.frag.size();
			info.pCode = reinterpret_cast<const uint32_t*>(shaderCode.frag.data());

			VkResult result = vkCreateShaderModule(context.GetDevice(), &info, nullptr, &fragShader);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return nullptr;
		}

		render::vk::VulkanShaderPass::ShaderModules shaderModules{};
		shaderModules.vert = vertShader;
		shaderModules.frag = fragShader;

		auto retShader = core::SObject::Create<render::vk::VulkanShaderPass>(context, shaderModules, passNode);
		retShader->StoreShaderCode(std::move(shaderCode));
		retShader->Build();

		return retShader;
	}
}