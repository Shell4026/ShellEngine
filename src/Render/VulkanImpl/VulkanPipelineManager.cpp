#include "pch.h"
#include "VulkanPipelineManager.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanVertexBuffer.h"

namespace sh::render::vk
{
	VulkanPipelineManager::VulkanPipelineManager(VkDevice device) :
		device(device)
	{
		shaderListener.SetCallback([&](core::SObject* obj)
			{
				auto shaderIt = shaderIdxs.find(static_cast<const VulkanShader*>(obj));
				if (shaderIt != shaderIdxs.end())
				{
					for (auto idx : shaderIt->second)
					{
						pipelines[idx][core::ThreadType::Game].reset();

						auto& info = pipelinesInfo[idx];
						if (auto infoIt = infoIdx.find(info); infoIt != infoIdx.end())
							infoIdx.erase(infoIt);
						if (auto meshIt = meshIdxs.find(info.mesh); meshIt != meshIdxs.end())
							meshIdxs.erase(meshIt);
					}
					shaderIdxs.erase(shaderIt);
				}
			}
		);
		meshListener.SetCallback([&](core::SObject* obj)
			{
				auto meshIt = meshIdxs.find(static_cast<const Mesh*>(obj));
				if (meshIt != meshIdxs.end())
				{
					for (auto idx : meshIt->second)
					{
						pipelines[idx][core::ThreadType::Game].reset();

						auto& info = pipelinesInfo[idx];
						if (auto infoIt = infoIdx.find(info); infoIt != infoIdx.end())
							infoIdx.erase(infoIt);
						if (auto shaderIt = shaderIdxs.find(info.shader); shaderIt != shaderIdxs.end())
							shaderIdxs.erase(shaderIt);
					}
					meshIdxs.erase(meshIt);
				}
			}
		);
	}

	auto VulkanPipelineManager::BuildPipeline(const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> std::unique_ptr<VulkanPipeline>
	{
		shader.onDestroy.Register(shaderListener);
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

	SH_RENDER_API auto VulkanPipelineManager::GetPipeline(core::ThreadType thr, const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> VulkanPipeline*
	{
		PipelineInfo info{ &pass, &shader, &mesh };
		auto it = infoIdx.find(info);
		if (it == infoIdx.end())
		{
			core::SyncArray<std::unique_ptr<VulkanPipeline>> syncArray{ nullptr, nullptr };
			syncArray[thr] = BuildPipeline(pass, shader, mesh);
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
			return pipelines[idx][thr].get();
		}
		else
		{
			VulkanPipeline* pipeline = pipelines[it->second][thr].get();
			if (pipeline == nullptr)
				pipelines[it->second][thr] = BuildPipeline(pass, shader, mesh);
			return pipelines[it->second][thr].get();
		}
	}
}//namespace