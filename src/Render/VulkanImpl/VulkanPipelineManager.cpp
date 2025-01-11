#include "VulkanPipelineManager.h"
#include "Mesh.h"
#include "Shader.h"
#include "VulkanShaderPass.h"
#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"

#include "Core/ThreadSyncManager.h"

namespace sh::render::vk
{
	VulkanPipelineManager::VulkanPipelineManager(const VulkanContext& context) :
		context(context),
		device(context.GetDevice())
	{
		meshListener.SetCallback([&](core::SObject* obj)
		{
			auto meshIt = meshIdxs.find(static_cast<const Mesh*>(obj));
			if (meshIt != meshIdxs.end())
			{
				for (auto idx : meshIt->second)
				{
					pipelines[idx][core::ThreadType::Game].reset();
					dirtyPipelines.push(idx);

					auto& info = pipelinesInfo[idx];
					if (auto infoIt = infoIdx.find(info); infoIt != infoIdx.end())
						infoIdx.erase(infoIt);
					if (auto shaderIt = shaderIdxs.find(info.shader); shaderIt != shaderIdxs.end())
						shaderIdxs.erase(shaderIt);
				}
				meshIdxs.erase(meshIt);
				SetDirty();
			}
		});
	}

	auto VulkanPipelineManager::BuildPipeline(const VkRenderPass& pass, VulkanShaderPass& shader, Mesh& mesh) -> std::unique_ptr<VulkanPipeline>
	{
		mesh.onDestroy.Register(meshListener);

		auto pipeline = std::make_unique<VulkanPipeline>(device, pass);

		VulkanPipeline::Topology topology = VulkanPipeline::Topology::Triangle;
		switch (mesh.GetTopology())
		{
		case Mesh::Topology::Point:
			topology = VulkanPipeline::Topology::Point;
			break;
		case Mesh::Topology::Line:
			topology = VulkanPipeline::Topology::Line;
			break;
		case Mesh::Topology::Face:
			topology = VulkanPipeline::Topology::Triangle;
			break;
		}

		pipeline->SetTopology(topology);
		pipeline->SetShader(&shader);
		pipeline->
			AddShaderStage(VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(VulkanPipeline::ShaderStage::Fragment);

		//Attribute
		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->attribDescriptions;

		// binding 0과 attribute 0은 버텍스
		for (int i = 0; i < attrs.size(); ++i)
		{
			auto data = shader.GetAttribute(mesh.attributes[i]->name);
			if (!data)
				continue;
			if (data->typeName != mesh.attributes[i]->typeName)
				continue;

			VkVertexInputAttributeDescription attrDesc = attrs[i];
			attrDesc.location = data->idx;

			pipeline->AddBindingDescription(bindings[i]);
			pipeline->AddAttributeDescription(attrDesc);
		}

		pipeline->SetLineWidth(mesh.lineWidth);

		auto result = pipeline->Build(shader.GetPipelineLayout());
		assert(result == VkResult::VK_SUCCESS);
		return pipeline;
	}

	SH_RENDER_API auto VulkanPipelineManager::GetPipelineHandle(const VkRenderPass& pass, VulkanShaderPass& shader, Mesh& mesh) -> uint64_t
	{
		PipelineInfo info{ &pass, &shader, &mesh };
		auto it = infoIdx.find(info);
		if (it == infoIdx.end())
		{
			core::SyncArray<std::unique_ptr<VulkanPipeline>> syncArray{ nullptr, nullptr };
			syncArray[core::ThreadType::Game] = BuildPipeline(pass, shader, mesh);
			syncArray[core::ThreadType::Render] = BuildPipeline(pass, shader, mesh);
			pipelines.push_back(std::move(syncArray));
			pipelinesInfo.push_back(info);

			std::size_t idx = pipelines.size() - 1;
			infoIdx.insert({ info, idx });

			if (auto it = renderpassIdxs.find(&pass); it == renderpassIdxs.end())
				renderpassIdxs.insert({ &pass, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			if (auto it = shaderIdxs.find(&shader); it == shaderIdxs.end())
				shaderIdxs.insert({ &shader, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);

			if (auto it = meshIdxs.find(&mesh); it == meshIdxs.end())
				meshIdxs.insert({ &mesh, std::vector<std::size_t>{idx} });
			else
				it->second.push_back(idx);
			return idx;
		}
		else
		{
			VulkanPipeline* pipeline = pipelines[it->second][core::ThreadType::Game].get();
			if (pipeline == nullptr)
				pipelines[it->second][core::ThreadType::Game] = BuildPipeline(pass, shader, mesh);

			pipeline = pipelines[it->second][core::ThreadType::Render].get();
			if (pipeline == nullptr)
				pipelines[it->second][core::ThreadType::Render] = BuildPipeline(pass, shader, mesh);

			return it->second;
		}
	}
	SH_RENDER_API void VulkanPipelineManager::BeginRender()
	{
		lastBindingPipeline = nullptr;
	}
	SH_RENDER_API bool VulkanPipelineManager::BindPipeline(VkCommandBuffer cmd, uint64_t handle)
	{
		VulkanPipeline* pipeline = pipelines[handle][core::ThreadType::Render].get();
		if (pipeline == nullptr)
			return false;

		if (lastBindingPipeline != pipeline)
		{
			vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
			lastBindingPipeline = pipeline;
		}
		return true;
	}
	SH_RENDER_API void sh::render::vk::VulkanPipelineManager::SetDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);

		bDirty = true;
	}
	SH_RENDER_API void sh::render::vk::VulkanPipelineManager::Sync()
	{
		while (!dirtyPipelines.empty())
		{
			uint64_t idx = dirtyPipelines.top();
			dirtyPipelines.pop();

			pipelines[idx][core::ThreadType::Render].reset();
		}
		bDirty = false;
	}
}//namespace