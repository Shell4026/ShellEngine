#include "VulkanComputePipeline.h"
#include "VulkanContext.h"

#include "Render/ComputeShader.h"

#include "Core/Logger.h"

#include <stdexcept>
#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API VulkanComputePipeline::VulkanComputePipeline(const VulkanContext& context, const ComputeShader& shader) :
		context(context),
		shader(&shader)
	{
	}

	SH_RENDER_API VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&& other) noexcept :
		context(other.context),
		shader(other.shader),
		shaderModule(other.shaderModule),
		setlayouts(std::move(other.setlayouts)),
		pipelineLayout(other.pipelineLayout),
		pipeline(other.pipeline)
	{
		other.shader = nullptr;
		other.shaderModule = VK_NULL_HANDLE;
		other.pipelineLayout = VK_NULL_HANDLE;
		other.pipeline = VK_NULL_HANDLE;
	}

	SH_RENDER_API VulkanComputePipeline::~VulkanComputePipeline()
	{
		Clean();
	}

	SH_RENDER_API void VulkanComputePipeline::Clean()
	{
		if (pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(context.GetDevice(), pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}
		CleanDescriptors();
		if (shaderModule != VK_NULL_HANDLE)
		{
			vkDestroyShaderModule(context.GetDevice(), shaderModule, nullptr);
			shaderModule = VK_NULL_HANDLE;
		}
	}

	SH_RENDER_API auto VulkanComputePipeline::Build() -> VkResult
	{
		assert(shader != nullptr);
		Clean();

		VkResult result = CreateShaderModule();
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreateShaderModule(): {}", string_VkResult(result));
			return result;
		}

		// 디스크립터 바인딩 수집
		for (const ShaderAST::BufferNode& buf : shader->GetNode().buffers)
		{
			VkDescriptorType type;
			switch (buf.bufferType)
			{
			case ShaderAST::BufferType::Storage:
				type = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case ShaderAST::BufferType::Uniform:
				type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				break;
			case ShaderAST::BufferType::Sampler:
				type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			case ShaderAST::BufferType::PushConstant:
				continue; // 푸시 상수는 디스크립터 아님
			default:
				continue;
			}
			AddDescriptorBinding(buf.set, buf.binding, type);
		}

		result = CreateDescriptorLayout();
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreateDescriptorLayout(): {}", string_VkResult(result));
			return result;
		}

		result = CreatePipelineLayout();
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreatePipelineLayout(): {}", string_VkResult(result));
			return result;
		}

		result = CreatePipeline();
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to CreateComputePipeline(): {}", string_VkResult(result));
			return result;
		}

		return VkResult::VK_SUCCESS;
	}

	SH_RENDER_API auto VulkanComputePipeline::GetDescriptorSetLayout(uint32_t set) const -> VkDescriptorSetLayout
	{
		if (set >= setlayouts.size())
			return VK_NULL_HANDLE;
		return setlayouts[set].descriptorSetLayout;
	}

	SH_RENDER_API auto VulkanComputePipeline::GetDescriptorBinding(uint32_t set, uint32_t binding) const -> std::optional<VkDescriptorSetLayoutBinding>
	{
		if (set >= setlayouts.size())
			return {};
		const SetLayout& setlayout = setlayouts[set];
		auto it = setlayout.descriptorSetLayoutBindings.find(binding);
		if (it == setlayout.descriptorSetLayoutBindings.end())
			return {};
		return it->second;
	}

	SH_RENDER_API auto VulkanComputePipeline::GetGroupSize() const -> std::array<uint32_t, 3>
	{
		if (shader == nullptr)
			return { 1, 1, 1 };
		return { shader->GetNumthreadsX(), shader->GetNumthreadsY(), shader->GetNumthreadsZ() };
	}

	auto VulkanComputePipeline::CreateShaderModule() -> VkResult
	{
		const std::vector<uint8_t>& spirv = shader->GetSpirv();

		VkShaderModuleCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = spirv.size();
		info.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

		return vkCreateShaderModule(context.GetDevice(), &info, nullptr, &shaderModule);
	}

	void VulkanComputePipeline::AddDescriptorBinding(uint32_t set, uint32_t binding, VkDescriptorType type)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = type;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;

		if (setlayouts.size() <= set)
			setlayouts.resize(set + 1);

		setlayouts[set].set = set;
		setlayouts[set].descriptorSetLayoutBindings.insert_or_assign(binding, layoutBinding);
	}

	auto VulkanComputePipeline::CreateDescriptorLayout() -> VkResult
	{
		VkResult result = VkResult::VK_SUCCESS;
		for (std::size_t i = 0; i < setlayouts.size(); ++i)
		{
			if (setlayouts[i].descriptorSetLayoutBindings.empty())
			{
				setlayouts[i].set = static_cast<uint32_t>(i);
				setlayouts[i].descriptorSetLayout = context.GetEmptyDescriptorSetLayout();
				continue;
			}
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.reserve(setlayouts[i].descriptorSetLayoutBindings.size());
			for (const auto& [binding, layoutBinding] : setlayouts[i].descriptorSetLayoutBindings)
				bindings.push_back(layoutBinding);

			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = static_cast<uint32_t>(bindings.size());
			info.pBindings = bindings.data();

			VkDescriptorSetLayout layout{};
			result = vkCreateDescriptorSetLayout(context.GetDevice(), &info, nullptr, &layout);
			if (result != VkResult::VK_SUCCESS)
				return result;

			setlayouts[i].descriptorSetLayout = layout;
		}
		return result;
	}

	auto VulkanComputePipeline::CreatePipelineLayout() -> VkResult
	{
		std::vector<VkDescriptorSetLayout> layouts(setlayouts.size(), VK_NULL_HANDLE);
		for (std::size_t i = 0; i < setlayouts.size(); ++i)
			layouts[i] = setlayouts[i].descriptorSetLayout;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		return vkCreatePipelineLayout(context.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	auto VulkanComputePipeline::CreatePipeline() -> VkResult
	{
		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = shaderModule;
		stageInfo.pName = "main";

		VkComputePipelineCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.stage = stageInfo;
		info.layout = pipelineLayout;
		info.basePipelineHandle = VK_NULL_HANDLE;
		info.basePipelineIndex = -1;

		return vkCreateComputePipelines(context.GetDevice(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
	}

	void VulkanComputePipeline::CleanDescriptors()
	{
		if (pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(context.GetDevice(), pipelineLayout, nullptr);
			pipelineLayout = VK_NULL_HANDLE;
		}
		for (std::size_t i = 0; i < setlayouts.size(); ++i)
		{
			if (setlayouts[i].descriptorSetLayout != VK_NULL_HANDLE &&
				setlayouts[i].descriptorSetLayout != context.GetEmptyDescriptorSetLayout())
			{
				vkDestroyDescriptorSetLayout(context.GetDevice(), setlayouts[i].descriptorSetLayout, nullptr);
			}
		}
		setlayouts.clear();
	}
}//namespace
