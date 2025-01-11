#include "VulkanShaderPass.h"

namespace sh::render::vk
{
	VulkanShaderPass::VulkanShaderPass(int id, VkDevice device) :
		ShaderPass(id, ShaderType::SPIR),
		device(device), vertShader(nullptr), fragShader(nullptr),
		descriptorSetLayout(1), pipelineLayout(nullptr)
	{
	}

	VulkanShaderPass::VulkanShaderPass(VulkanShaderPass&& other) noexcept :
		ShaderPass(std::move(other)),
		device(other.device),
		vertShader(other.vertShader), fragShader(other.fragShader),
		descriptorBindings(std::move(other.descriptorBindings)),
		localDescriptorBindings(std::move(other.localDescriptorBindings)),
		descriptorSetLayout(std::move(other.descriptorSetLayout)),
		pipelineLayout(other.pipelineLayout)
	{
		other.vertShader = nullptr;
		other.fragShader = nullptr;
		other.device = nullptr;
		other.pipelineLayout = nullptr;
	}

	VulkanShaderPass::~VulkanShaderPass()
	{
		Clean();
	}
	SH_RENDER_API void VulkanShaderPass::Clean()
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
	void VulkanShaderPass::CleanDescriptors()
	{
		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}

		localDescriptorBindings.clear();
		descriptorBindings.clear();

		for (std::size_t i = 0; i < descriptorSetLayout.size(); ++i)
		{
			if (descriptorSetLayout[i])
				vkDestroyDescriptorSetLayout(device, descriptorSetLayout[i], nullptr);
		}
		descriptorSetLayout.clear();
		descriptorSetLayout.resize(2);
	}

	SH_RENDER_API void VulkanShaderPass::SetVertexShader(VkShaderModule shader)
	{
		vertShader = shader;
	}
	SH_RENDER_API auto VulkanShaderPass::GetVertexShader() const -> const VkShaderModule
	{
		return vertShader;
	}

	SH_RENDER_API void VulkanShaderPass::SetFragmentShader(VkShaderModule shader)
	{
		fragShader = shader;
	}
	SH_RENDER_API auto VulkanShaderPass::GetFragmentShader() const -> const VkShaderModule
	{
		return fragShader;
	}

	void VulkanShaderPass::AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = type;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = stage;

		if (set == 0)
			localDescriptorBindings.push_back(layoutBinding);
		else
			descriptorBindings.push_back(layoutBinding);
	}
	auto VulkanShaderPass::CreateDescriptorLayout() -> VkResult
	{
		if (descriptorSetLayout.size() == 1)
			descriptorSetLayout.push_back(nullptr);

		VkResult result = VkResult::VK_SUCCESS;
		{
			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = localDescriptorBindings.size();
			info.pBindings = localDescriptorBindings.data();

			VkDescriptorSetLayout localLayout{};
			result = vkCreateDescriptorSetLayout(device, &info, nullptr, &localLayout);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Failed to create local descriptor layout!: "} + string_VkResult(result) };

			descriptorSetLayout[0] = localLayout;
		}
		{
			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = descriptorBindings.size();
			info.pBindings = descriptorBindings.data();

			VkDescriptorSetLayout layout{};
			result = vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Failed to create descriptor layout!: "} + string_VkResult(result) };

			descriptorSetLayout[1] = layout;
		}
		return result;
	}
	auto VulkanShaderPass::CreatePipelineLayout() -> VkResult
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayout.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0; // 푸시 상수 쓸거면 수정
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		return vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	SH_RENDER_API void VulkanShaderPass::Build()
	{
		CleanDescriptors();
		// 유니폼 레이아웃 생성
		for (auto& uniform : vertexUniforms)
		{
			uint32_t set = uniform.type == ShaderPass::UniformType::Object ? 0 : 1;
			AddDescriptorBinding(set, uniform.binding,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
		}
		for (auto& uniform : fragmentUniforms)
		{
			uint32_t set = uniform.type == ShaderPass::UniformType::Object ? 0 : 1;
			AddDescriptorBinding(set, uniform.binding,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		for (auto& uniform : samplerUniforms)
		{
			AddDescriptorBinding(1, uniform.binding,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		auto result = CreateDescriptorLayout();
		assert(result == VkResult::VK_SUCCESS);

		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);
	}

	SH_RENDER_API auto VulkanShaderPass::GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout
	{
		return descriptorSetLayout[set];
	}
	SH_RENDER_API auto VulkanShaderPass::GetPipelineLayout() const -> VkPipelineLayout
	{
		return pipelineLayout;
	}
}