#include "VulkanShader.h"

#include <cassert>

namespace sh::render
{
	VulkanShader::VulkanShader(int id, VkDevice device) :
		Shader(id, ShaderType::SPIR),
		device(device), vertShader(nullptr), fragShader(nullptr),
		descriptorSetLayout(nullptr), pipelineLayout(nullptr)
	{
	}

	VulkanShader::VulkanShader(VulkanShader&& other) noexcept :
		Shader(std::move(other)),
		device(device),
		vertShader(other.vertShader), fragShader(other.fragShader),
		descriptorSetLayout(other.descriptorSetLayout), pipelineLayout(other.pipelineLayout)
	{
		other.vertShader = nullptr;
		other.fragShader = nullptr;
		other.device = nullptr;
		other.descriptorSetLayout = nullptr;
		other.pipelineLayout = nullptr;
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
		CleanDescriptors();

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
	void VulkanShader::CleanDescriptors()
	{
		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}

		descriptorBindings.clear();

		if (descriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
			descriptorSetLayout = nullptr;
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

	void VulkanShader::AddDescriptorBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = type;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = stage;
		descriptorBindings.push_back(layoutBinding);
	}
	auto VulkanShader::CreateDescriptorLayout() -> VkResult
	{
		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = descriptorBindings.size();
		info.pBindings = descriptorBindings.data();

		return vkCreateDescriptorSetLayout(device, &info, nullptr, &descriptorSetLayout);
	}
	auto VulkanShader::CreatePipelineLayout() -> VkResult
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	void VulkanShader::Build()
	{
		CleanDescriptors();
		//유니폼 버퍼, 바인딩 생성
		for (auto& uniform : vertexUniforms)
		{
			AddDescriptorBinding(uniform.first,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
		}
		for (auto& uniform : fragmentUniforms)
		{
			AddDescriptorBinding(uniform.first,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		for (auto& uniform : samplerFragmentUniforms)
		{
			AddDescriptorBinding(uniform.first,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		auto result = CreateDescriptorLayout();
		assert(result == VkResult::VK_SUCCESS);
		
		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);
	}

	auto VulkanShader::GetDescriptorSetLayout() const -> VkDescriptorSetLayout
	{
		return descriptorSetLayout;
	}
	auto VulkanShader::GetPipelineLayout() const -> VkPipelineLayout
	{
		return pipelineLayout;
	}
}