#include "VulkanShaderBuilder.h"

#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"
#include "VulkanImpl/VulkanFramebuffer.h"

#include <cassert>

namespace sh::render
{
	VulkanShaderBuilder::VulkanShaderBuilder(VulkanRenderer& renderer) :
		renderer(renderer)
	{
	}

	VulkanShaderBuilder::~VulkanShaderBuilder()
	{
	}

	auto VulkanShaderBuilder::Build() -> std::unique_ptr<Shader>
	{
		assert(renderer.GetDevice() != nullptr);
		VkShaderModule vertShader{nullptr}, fragShader{nullptr};

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

		std::unique_ptr<Shader> retShader = std::unique_ptr<Shader>{ new VulkanShader{GetNextId(), renderer.GetDevice()}};
		VulkanShader* shader = static_cast<VulkanShader*>(retShader.get());
		shader->SetVertexShader(vertShader);
		shader->SetFragmentShader(fragShader);
		
		auto renderPass = static_cast<impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer())->GetRenderPass();
		shader->pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), renderPass, shader);
		shader->pipeline->
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment);
		if (shader->pipeline->Build() != VkResult::VK_SUCCESS)
			return nullptr;

		return retShader;
	}
}