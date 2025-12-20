#include "VulkanPipelineManager.h"
#include "VulkanShaderPass.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

namespace sh::render::vk
{
	VulkanPipelineManager::VulkanPipelineManager(const VulkanContext& context) :
		context(context)
	{
		shaderDestroyedListener.SetCallback(
			[&](const core::SObject* obj)
			{
				auto shaderPassPtr = static_cast<const VulkanShaderPass*>(obj);

				std::unique_lock writeLock{ mu };

				auto it = shaderIdxs.find(shaderPassPtr);
				if (it == shaderIdxs.end())
					return;

				for (auto idx : it->second)
				{
					if (idx < pipelines.size())
					{
						pipelines[idx].pipelinePtr.reset();
						emptyIdx.push(idx);
					}
				}

				shaderIdxs.erase(it);

				for (auto it = infoIdx.begin(); it != infoIdx.end(); )
				{
					if (it->first.shader == shaderPassPtr)
						it = infoIdx.erase(it);
					else
						++it;
				}
			}
		);
	}
	SH_RENDER_API auto VulkanPipelineManager::GetOrCreatePipelineHandle(
		const VulkanShaderPass& shader,
		const RenderTargetLayout& renderTargetLayout,
		Mesh::Topology topology,
		const std::vector<uint8_t>* constDataPtr) -> PipelineHandle
	{
		std::size_t constantHash = 0;
		if (constDataPtr != nullptr)
		{
			std::hash<uint8_t> hasher;
			for (uint8_t data : *constDataPtr)
				constantHash = core::Util::CombineHash(constantHash, hasher(data));
		}

		PipelineInfo info{ &shader, renderTargetLayout, topology, constantHash };
		{
			std::shared_lock<std::shared_mutex> readLock{ mu };
			auto it = infoIdx.find(info);
			if (it != infoIdx.end())
			{
				VulkanPipeline* pipeline = pipelines[it->second].pipelinePtr.get();
				uint32_t gen = pipelines[it->second].generation;
				if (pipeline == nullptr)
				{
					readLock.unlock();
					std::unique_lock<std::shared_mutex> writeLock{ mu };
					if (pipelines[it->second].pipelinePtr.get() == nullptr)
					{
						pipelines[it->second].pipelinePtr = BuildPipeline(shader, renderTargetLayout, topology, constDataPtr);
						gen = ++pipelines[it->second].generation;
					}
				}
				return PipelineHandle{ it->second, gen };
			}
		}
		{
			std::unique_lock<std::shared_mutex> writeLock{ mu };
			auto it = infoIdx.find(info);
			if (it != infoIdx.end())
				return PipelineHandle{ it->second, pipelines[it->second].generation };

			shader.onDestroy.Register(shaderDestroyedListener);

			uint32_t idx = 0;
			uint32_t gen = 0;
			if (emptyIdx.empty())
			{
				pipelines.push_back(Pipeline{ BuildPipeline(shader, renderTargetLayout, topology, constDataPtr), 0 });
				pipelinesInfo.push_back(info);

				idx = static_cast<uint32_t>(pipelines.size()) - 1;
			}
			else
			{
				idx = emptyIdx.front();
				emptyIdx.pop();

				pipelines[idx].pipelinePtr = BuildPipeline(shader, renderTargetLayout, topology, constDataPtr);
				gen = ++pipelines[idx].generation;
			}
			infoIdx.insert({ info, idx });

			if (auto it = shaderIdxs.find(&shader); it == shaderIdxs.end())
				shaderIdxs.insert({ &shader, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			return PipelineHandle{ idx, gen };
		}
	}
	SH_RENDER_API bool VulkanPipelineManager::BindPipeline(VkCommandBuffer cmd, PipelineHandle handle)
	{
		VkPipeline vkPipeline = VK_NULL_HANDLE;
		{
			std::shared_lock readLock{ mu };
			if (handle.index >= pipelines.size())
				return false;

			auto& slot = pipelines[handle.index];
			if (slot.generation != handle.generation) 
				return false;
			if (!slot.pipelinePtr) 
				return false;

			vkPipeline = slot.pipelinePtr->GetPipeline();
		}
		vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
		return true;
	}

	auto VulkanPipelineManager::BuildPipeline(
		const VulkanShaderPass& shader, 
		const RenderTargetLayout& renderTargetLayout, 
		Mesh::Topology topology, 
		const std::vector<uint8_t>* constDataPtr) -> std::unique_ptr<VulkanPipeline>
	{
		auto pipeline = std::make_unique<VulkanPipeline>(context.GetDevice(), renderTargetLayout);

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
			SetSampleCount(renderTargetLayout.bUseMSAA ? context.GetSampleCount() : VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT).
			AddShaderStage(VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(VulkanPipeline::ShaderStage::Fragment);

		if (constDataPtr != nullptr)
			pipeline->SetSpecializationConstant(constDataPtr->data());

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
}//namespace