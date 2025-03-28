#include "VulkanShaderPass.h"
#include "VulkanContext.h"

namespace sh::render::vk
{
	VulkanShaderPass::VulkanShaderPass(const VulkanContext& context, const ShaderModules& shaderModules, const ShaderAST::PassNode& passNode) :
		ShaderPass(passNode, ShaderType::SPIR),
		context(context),
		vertShader(shaderModules.vert), fragShader(shaderModules.frag),
		descriptorSetLayout(1), pipelineLayout(nullptr)
	{
		for (auto& stage : passNode.stages)
		{
			for (auto& uniform : stage.uniforms)
			{
				if (uniform.bConstant)
				{
					bUseMatrixModel = true;
					return;
				}
			}
		}
	}

	VulkanShaderPass::VulkanShaderPass(VulkanShaderPass&& other) noexcept :
		ShaderPass(std::move(other)),
		context(other.context),
		vertShader(other.vertShader), fragShader(other.fragShader),
		descriptorBindings(std::move(other.descriptorBindings)),
		descriptorSetLayout(std::move(other.descriptorSetLayout)),
		pipelineLayout(other.pipelineLayout),
		bUseMatrixModel(other.bUseMatrixModel)
	{
		other.vertShader = nullptr;
		other.fragShader = nullptr;
		other.pipelineLayout = nullptr;
	}

	VulkanShaderPass::~VulkanShaderPass()
	{
		Clear();
	}
	SH_RENDER_API void VulkanShaderPass::Clear()
	{
		CleanDescriptors();

		if (vertShader)
		{
			vkDestroyShaderModule(context.GetDevice(), vertShader, nullptr);
			vertShader = nullptr;
		}
		if (fragShader)
		{
			vkDestroyShaderModule(context.GetDevice(), fragShader, nullptr);
			fragShader = nullptr;
		}
	}
	void VulkanShaderPass::CleanDescriptors()
	{
		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(context.GetDevice(), pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}

		descriptorBindings.clear();

		for (std::size_t i = 0; i < descriptorSetLayout.size(); ++i)
		{
			if (descriptorSetLayout[i] != context.GetEmptyDescriptorSetLayout())
				vkDestroyDescriptorSetLayout(context.GetDevice(), descriptorSetLayout[i], nullptr);
		}
		descriptorSetLayout.clear();
	}

	SH_RENDER_API auto VulkanShaderPass::GetVertexShader() const -> const VkShaderModule
	{
		return vertShader;
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

		if (descriptorBindings.size() <= set)
			descriptorBindings.resize(set + 1, core::SVector<VkDescriptorSetLayoutBinding>());

		descriptorBindings[set].push_back(layoutBinding);
	}
	auto VulkanShaderPass::CreateDescriptorLayout() -> VkResult
	{
		descriptorSetLayout.resize(descriptorBindings.size());

		VkResult result = VkResult::VK_SUCCESS;
		for (int i = 0; i < descriptorBindings.size(); ++i)
		{
			if (descriptorBindings[i].empty())
			{
				descriptorSetLayout[i] = context.GetEmptyDescriptorSetLayout();
				continue;
			}
			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = descriptorBindings[i].size();
			info.pBindings = descriptorBindings[i].data();

			VkDescriptorSetLayout layout{};
			result = vkCreateDescriptorSetLayout(context.GetDevice(), &info, nullptr, &layout);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Failed to create descriptor layout!: "} + string_VkResult(result) };

			descriptorSetLayout[i] = layout;
		}
		return result;
	}
	auto VulkanShaderPass::CreatePipelineLayout() -> VkResult
	{
		// 푸쉬 상수는 모델 행렬 전달 할 때만 씀
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(glm::mat4);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = descriptorSetLayout.size();
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
		pipelineLayoutInfo.pushConstantRangeCount = bUseMatrixModel ? 1 : 0;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		return vkCreatePipelineLayout(context.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	SH_RENDER_API void VulkanShaderPass::Build()
	{
		CleanDescriptors();
		// 유니폼 레이아웃 생성
		for (auto& uniformLayout : vertexUniforms)
		{
			if (uniformLayout.bConstant)
				continue;
			uint32_t set = static_cast<uint32_t>(uniformLayout.type);

			VkDescriptorType type = (set == 0) ? 
				VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			
			AddDescriptorBinding(set, uniformLayout.binding, type,
				VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
		}
		for (auto& uniformLayout : fragmentUniforms)
		{
			if (uniformLayout.bConstant)
				continue;
			uint32_t set = static_cast<uint32_t>(uniformLayout.type);

			VkDescriptorType type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			if (set == 0)
				type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			
			AddDescriptorBinding(set, uniformLayout.binding, type,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		for (auto& uniformLayout : samplerUniforms)
		{
			uint32_t set = static_cast<uint32_t>(uniformLayout.type);
			AddDescriptorBinding(set, uniformLayout.binding,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		auto result = CreateDescriptorLayout();
		assert(result == VkResult::VK_SUCCESS);

		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);
	}

	SH_RENDER_API auto VulkanShaderPass::GetDescriptorSetLayout(uint32_t set) const -> DescryptorSetLayoutInfo
	{
		DescryptorSetLayoutInfo info{};
		info.layout = descriptorSetLayout[set];
		info.bDynamic = (set == 0);
		return info;
	}
	SH_RENDER_API auto VulkanShaderPass::GetSetCount() const -> uint32_t
	{
		return static_cast<uint32_t>(descriptorSetLayout.size());
	}
	SH_RENDER_API auto VulkanShaderPass::GetPipelineLayout() const -> VkPipelineLayout
	{
		return pipelineLayout;
	}
}