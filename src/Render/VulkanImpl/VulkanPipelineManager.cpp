#include "VulkanPipelineManager.h"
#include "VulkanShaderPass.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"

namespace sh::render::vk
{
	VulkanPipelineManager::VulkanPipelineManager(const VulkanContext& context) :
		context(context), device(context.GetDevice())
	{
		// TODO: pipelines에 nullptr가 많아지면 어떻게 할지 생각해두기
		shaderDestroyedListener.SetCallback(
			[&](const core::SObject* obj)
			{
				auto shaderPassPtr = static_cast<const VulkanShaderPass*>(obj);
				std::shared_lock<std::shared_mutex> readLock{ mu };
				auto it = shaderIdxs.find(shaderPassPtr);
				if (it != shaderIdxs.end())
				{
					auto& idxs = it->second;
					readLock.unlock();

					std::unique_lock<std::shared_mutex> writeLock{ mu };

					for (std::size_t idx : idxs)
						pipelines[idx] = nullptr;
					shaderIdxs.erase(it);

					for (auto it = infoIdx.begin(); it != infoIdx.end();)
					{
						auto& info = it->first;
						if (info.shader == shaderPassPtr)
							it = infoIdx.erase(it);
						else
							++it;
					}
				}
			}
		);
	}

	VulkanPipelineManager::VulkanPipelineManager(VulkanPipelineManager&& other) noexcept :
		context(other.context), device(other.device),
		pipelines(std::move(other.pipelines)), pipelinesInfo(std::move(other.pipelinesInfo)),
		infoIdx(std::move(other.infoIdx)), renderpassIdxs(std::move(other.renderpassIdxs)), shaderIdxs(std::move(other.shaderIdxs))
	{
	}

	auto VulkanPipelineManager::BuildPipeline(const VulkanRenderPass& renderPass, const VulkanShaderPass& shader, Mesh::Topology topology) -> std::unique_ptr<VulkanPipeline>
	{
		auto pipeline = std::make_unique<VulkanPipeline>(device, renderPass.GetVkRenderPass());

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
			SetSampleCount(renderPass.GetConfig().sampleCount);
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

	SH_RENDER_API auto VulkanPipelineManager::GetOrCreatePipelineHandle(const VulkanRenderPass& renderPass, const VulkanShaderPass& shader, Mesh::Topology topology) -> uint64_t
	{
		PipelineInfo info{ renderPass.GetVkRenderPass(), &shader, topology };
		{
			std::shared_lock<std::shared_mutex> readLock{ mu };
			auto it = infoIdx.find(info);
			if (it != infoIdx.end())
			{
				VulkanPipeline* pipeline = pipelines[it->second].get();
				if (pipeline == nullptr)
				{
					readLock.unlock();
					std::unique_lock<std::shared_mutex> writeLock{ mu };
					if (pipelines[it->second].get() == nullptr)
						pipelines[it->second] = BuildPipeline(renderPass, shader, topology);
				}
				return it->second;
			}
		}
		{
			std::unique_lock<std::shared_mutex> writeLock{ mu };
			auto it = infoIdx.find(info);
			if (it != infoIdx.end())
				return it->second;

			shader.onDestroy.Register(shaderDestroyedListener);

			pipelines.push_back(BuildPipeline(renderPass, shader, topology));
			pipelinesInfo.push_back(info);

			std::size_t idx = pipelines.size() - 1;
			infoIdx.insert({ info, idx });

			if (auto it = renderpassIdxs.find(renderPass.GetVkRenderPass()); it == renderpassIdxs.end())
				renderpassIdxs.insert({ renderPass.GetVkRenderPass(), std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			if (auto it = shaderIdxs.find(&shader); it == shaderIdxs.end())
				shaderIdxs.insert({ &shader, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			return idx;
		}
	}
	SH_RENDER_API bool VulkanPipelineManager::BindPipeline(VkCommandBuffer cmd, uint64_t handle)
	{
		std::shared_lock<std::shared_mutex> readLock{ mu };
		VulkanPipeline* pipeline = pipelines[handle].get();
		if (pipeline == nullptr)
			return false;

		vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
		return true;
	}
}//namespace