#include "VulkanShaderBuilder.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanShader.h"
#include "Render/VulkanImpl/VulkanPipeline.h"
#include "Render/VulkanImpl/VulkanFramebuffer.h"

#include <cassert>

namespace sh::game
{
	VulkanShaderBuilder::VulkanShaderBuilder(render::VulkanRenderer& renderer) :
		renderer(renderer)
	{
	}

	VulkanShaderBuilder::~VulkanShaderBuilder()
	{
	}

	auto VulkanShaderBuilder::Build() -> render::Shader*
	{
		assert(renderer.GetDevice() != nullptr);
		VkShaderModule vertShader{ nullptr }, fragShader{ nullptr };

		assert(vertShaderData.data());
		assert(fragShaderData.data());
		//Vertex shader
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = vertShaderData.size();
		info.pCode = reinterpret_cast<const uint32_t*>(vertShaderData.data());

		VkResult result = vkCreateShaderModule(renderer.GetDevice(), &info, nullptr, &vertShader);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return nullptr;

		//Fragment shader
		info.codeSize = fragShaderData.size();
		info.pCode = reinterpret_cast<const uint32_t*>(fragShaderData.data());

		result = vkCreateShaderModule(renderer.GetDevice(), &info, nullptr, &fragShader);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return nullptr;

		auto retShader = core::SObject::Create<render::VulkanShader>(GetNextId(), renderer.GetDevice());

		render::VulkanShader* shader = static_cast<render::VulkanShader*>(retShader);
		shader->SetVertexShader(vertShader);
		shader->SetFragmentShader(fragShader);

		return retShader;
	}
}