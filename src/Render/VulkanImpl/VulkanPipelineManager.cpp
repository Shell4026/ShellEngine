#include "VulkanPipelineManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanShaderPass.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

namespace sh::render::vk
{
	VulkanPipelineManager::VulkanPipelineManager(const VulkanContext& context) :
		context(context), device(context.GetDevice())
	{
	}

	VulkanPipelineManager::VulkanPipelineManager(VulkanPipelineManager&& other) noexcept :
		context(other.context), device(other.device),
		pipelines(std::move(other.pipelines)), pipelinesInfo(std::move(other.pipelinesInfo)),
		infoIdx(std::move(other.infoIdx)), renderpassIdxs(std::move(other.renderpassIdxs)), shaderIdxs(std::move(other.shaderIdxs))
	{
	}

	auto VulkanPipelineManager::BuildPipeline(VkRenderPass renderPass, VulkanShaderPass& shader, Mesh::Topology topology) -> std::unique_ptr<VulkanPipeline>
	{
		auto pipeline = std::make_unique<VulkanPipeline>(device, renderPass);

		VulkanPipeline::Topology topol = VulkanPipeline::Topology::Triangle;
		switch (topology)
		{
		case Mesh::Topology::Point:
			topol = VulkanPipeline::Topology::Point;
			break;
		case Mesh::Topology::Line:
			topol = VulkanPipeline::Topology::Line;
			break;
		case Mesh::Topology::Face:
			topol = VulkanPipeline::Topology::Triangle;
			break;
		}

		pipeline->
			SetCullMode(shader.GetCullMode()).
			SetTopology(topol).
			SetShader(&shader).
			SetZWrite(shader.GetZWrite()).
			SetSampleCount(context.GetSampleCount());
		pipeline->
			AddShaderStage(VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(VulkanPipeline::ShaderStage::Fragment);

		pipeline->AddBindingDescription(VulkanVertexBuffer::GetBindingDescription());
		for (auto& attrDesc : VulkanVertexBuffer::GetAttributeDescriptions())
			pipeline->AddAttributeDescription(attrDesc);

		pipeline->SetLineWidth(1.0f);
		pipeline->SetStencilState(true, ConvertStencilState(shader.GetStencilState()));

		auto result = pipeline->Build(shader.GetPipelineLayout());
		assert(result == VkResult::VK_SUCCESS);
		return pipeline;
	}

	auto VulkanPipelineManager::ConvertStencilState(const StencilState& stencilState) const -> VkStencilOpState
	{
		VkStencilOpState state{};
		state.reference = stencilState.ref;
		state.compareMask = stencilState.compareMask;
		state.writeMask = stencilState.writeMask;
		switch (stencilState.compareOp)
		{
		case StencilState::CompareOp::Always:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS; break;
		case StencilState::CompareOp::Equal:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_EQUAL; break;
		case StencilState::CompareOp::Greater:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_GREATER; break;
		case StencilState::CompareOp::GreaterEqual:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL; break;
		case StencilState::CompareOp::Less:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_LESS; break;
		case StencilState::CompareOp::LessEqual:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL; break;
		case StencilState::CompareOp::Never:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER; break;
		case StencilState::CompareOp::NotEqual:
			state.compareOp = VkCompareOp::VK_COMPARE_OP_NOT_EQUAL; break;
		}
		auto fn = 
			[&](StencilState::StencilOp stencilOp) -> VkStencilOp
			{
				switch (stencilOp)
				{
				case StencilState::StencilOp::Zero:
					return VkStencilOp::VK_STENCIL_OP_ZERO;
				case StencilState::StencilOp::Replace:
					return VkStencilOp::VK_STENCIL_OP_REPLACE;
				case StencilState::StencilOp::Keep:
					return VkStencilOp::VK_STENCIL_OP_KEEP;
				case StencilState::StencilOp::Invert:
					return VkStencilOp::VK_STENCIL_OP_INVERT;
				case StencilState::StencilOp::IncrementClamp:
					return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP;
				case StencilState::StencilOp::IncrementWrap:
					return VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP;
				case StencilState::StencilOp::DecrementClamp:
					return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP;
				case StencilState::StencilOp::DecrementWrap:
					return VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP;
				}
				return VkStencilOp::VK_STENCIL_OP_ZERO;
			};
		state.passOp = fn(stencilState.passOp);
		state.failOp = fn(stencilState.failOp);
		state.depthFailOp = fn(stencilState.depthFailOp);
		return state;
	}

	SH_RENDER_API auto VulkanPipelineManager::GetOrCreatePipelineHandle(VkRenderPass renderPass, VulkanShaderPass& shader, Mesh::Topology topology) -> uint64_t
	{
		PipelineInfo info{ renderPass, &shader, topology };
		auto it = infoIdx.find(info);
		if (it == infoIdx.end())
		{
			pipelines.push_back(BuildPipeline(renderPass, shader, topology));
			pipelinesInfo.push_back(info);

			std::size_t idx = pipelines.size() - 1;
			infoIdx.insert({ info, idx });

			if (auto it = renderpassIdxs.find(renderPass); it == renderpassIdxs.end())
				renderpassIdxs.insert({ renderPass, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			if (auto it = shaderIdxs.find(&shader); it == shaderIdxs.end())
				shaderIdxs.insert({ &shader, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			return idx;
		}
		else
		{
			VulkanPipeline* pipeline = pipelines[it->second].get();
			if (pipeline == nullptr)
				pipelines[it->second] = BuildPipeline(renderPass, shader, topology);

			return it->second;
		}
	}
	SH_RENDER_API bool VulkanPipelineManager::BindPipeline(VkCommandBuffer cmd, uint64_t handle)
	{
		VulkanPipeline* pipeline = pipelines[handle].get();
		if (pipeline == nullptr)
			return false;

		vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
		return true;
	}
}//namespace