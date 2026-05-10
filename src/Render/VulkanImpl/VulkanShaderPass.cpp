#include "VulkanShaderPass.h"
#include "VulkanContext.h"

#include "Core/Logger.h"

#include <stdexcept>

namespace sh::render::vk
{
	VulkanShaderPass::VulkanShaderPass(const VulkanContext& context, const ShaderModules& shaderModules, const ShaderAST::PassNode& passNode) :
		ShaderPass(passNode, ShaderType::SPIR),
		context(context),
		vertShader(shaderModules.vert), fragShader(shaderModules.frag),
		pipelineLayout(nullptr)
	{
		// Specialization constant (컴파일 상수)
		if (!passNode.constants.empty())
		{
			specializationEntry.reserve(passNode.constants.size());

			std::size_t cursor = 0;
			for (int i = 0; i < passNode.constants.size(); ++i)
			{
				const auto& varNode = passNode.constants[i];

				VkSpecializationMapEntry entry{};
				entry.constantID = i;
				switch (varNode.type)
				{
				case ShaderAST::VariableType::Boolean: [[fallthrough]];
				case ShaderAST::VariableType::Int: [[fallthrough]];
				case ShaderAST::VariableType::Float:
					entry.size = 4;
					break;
				}
				entry.offset = cursor;
				cursor += entry.size;
				specializationEntry.push_back(entry);
			}
		}
		for (auto& stage : passNode.stages)
		{
			for (auto& uniform : stage.buffers)
			{
				if (uniform.bufferType == ShaderAST::BufferType::PushConstant)
				{
					bUseMatrixModel = true;
					break;
				}
			}
			if (bUseMatrixModel)
				break;
		}
	}

	VulkanShaderPass::VulkanShaderPass(VulkanShaderPass&& other) noexcept :
		ShaderPass(std::move(other)),
		context(other.context),
		vertShader(other.vertShader), fragShader(other.fragShader),
		setlayouts(std::move(other.setlayouts)),
		pipelineLayout(other.pipelineLayout),
		specializationEntry(std::move(other.specializationEntry)),
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
		specializationEntry.clear();

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
	SH_RENDER_API void VulkanShaderPass::Build()
	{
		CleanDescriptors();
		// 유니폼 레이아웃 생성
		for (const std::vector<UniformStructLayout>* const uniforms : { &vertexUniforms, &fragmentUniforms })
		{
			for (const UniformStructLayout& uniformLayout : *uniforms)
			{
				if (uniformLayout.IsPushConstant())
					continue;
				const uint32_t set = static_cast<uint32_t>(uniformLayout.usage);

				// set == 0 카메라 데이터
				VkDescriptorType type = (set == 0) ?
					VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				if (uniformLayout.GetKind() == UniformStructLayout::Kind::Storage)
					type = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

				VkShaderStageFlagBits stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
				if (uniforms == &fragmentUniforms)
					stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;

				AddDescriptorBinding(set, uniformLayout.binding, type, stage);
			}
		}
		for (const UniformStructLayout& uniformLayout : samplerUniforms)
		{
			const uint32_t set = static_cast<uint32_t>(uniformLayout.usage);
			AddDescriptorBinding(set, uniformLayout.binding,
				VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
		}
		VkResult result = CreateDescriptorLayout();
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreateDescriptorLayout(): {}", string_VkResult(result));
			throw std::runtime_error{ "Failed to CreateDescriptorLayout()" };
		}

		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreatePipelineLayout(): {}", string_VkResult(result));
			throw std::runtime_error{ "Failed to CreatePipelineLayout()" };
		}
	}

	SH_RENDER_API auto VulkanShaderPass::GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout
	{
		if (setlayouts.size() < set)
			return VK_NULL_HANDLE;
		return setlayouts[set].descriptorSetLayout;
	}

	SH_RENDER_API auto VulkanShaderPass::GetSetLayout(uint32_t set) const -> std::optional<SetLayout>
	{
		if (setlayouts.size() < set)
			return {};
		return setlayouts[set];
	}

	SH_RENDER_API auto VulkanShaderPass::GetDescriptorBinding(uint32_t set, uint32_t binding) const -> std::optional<VkDescriptorSetLayoutBinding>
	{
		if (setlayouts.size() < set)
			return {};
		const SetLayout& setlayout = setlayouts[set];
		auto it = setlayout.descriptorSetLayoutBindings.find(binding);
		if (it == setlayout.descriptorSetLayoutBindings.end())
			return{};
		return it->second;
	}

	void VulkanShaderPass::AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlagBits stage)
	{
		if (setlayouts.size() <= set)
		{
			setlayouts.resize(set + 1);
		}
		setlayouts[set].set = set;

		auto it = setlayouts[set].descriptorSetLayoutBindings.find(binding);
		if (it != setlayouts[set].descriptorSetLayoutBindings.end())
		{
			assert(it->second.descriptorType == type);
			it->second.stageFlags |= stage;
			return;
		}

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = type;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = stage;

		setlayouts[set].descriptorSetLayoutBindings.insert_or_assign(binding, layoutBinding);
	}
	auto VulkanShaderPass::CreateDescriptorLayout() -> VkResult
	{
		VkResult result = VkResult::VK_SUCCESS;
		for (int i = 0; i < setlayouts.size(); ++i)
		{
			if (setlayouts[i].descriptorSetLayoutBindings.empty())
			{
				setlayouts[i].set = i;
				setlayouts[i].descriptorSetLayout = context.GetEmptyDescriptorSetLayout();
				continue;
			}
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.reserve(setlayouts[i].descriptorSetLayoutBindings.size());
			for (const auto& [binding, descriptorSetLayoytBinding] : setlayouts[i].descriptorSetLayoutBindings)
				bindings.push_back(descriptorSetLayoytBinding);

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = bindings.size();
			info.pBindings = bindings.data();

			VkDescriptorSetLayout layout{};
			result = vkCreateDescriptorSetLayout(context.GetDevice(), &info, nullptr, &layout);
			if (result != VkResult::VK_SUCCESS)
				return result;

			setlayouts[i].descriptorSetLayout = layout;
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

		std::vector<VkDescriptorSetLayout> layouts(setlayouts.size(), VK_NULL_HANDLE);
		for (int i = 0; i < setlayouts.size(); ++i)
			layouts[i] = setlayouts[i].descriptorSetLayout;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = bUseMatrixModel ? 1 : 0;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		return vkCreatePipelineLayout(context.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}
	void VulkanShaderPass::CleanDescriptors()
	{
		if (pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(context.GetDevice(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}

		for (std::size_t i = 0; i < setlayouts.size(); ++i)
		{
			if (setlayouts[i].descriptorSetLayout != context.GetEmptyDescriptorSetLayout())
				vkDestroyDescriptorSetLayout(context.GetDevice(), setlayouts[i].descriptorSetLayout, nullptr);
		}
		setlayouts.clear();
	}
}//namespace
