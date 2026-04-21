#include "MaterialData.h"
#include "Shader.h"
#include "ShaderPass.h"
#include "BufferFactory.h"
#include "IRenderContext.h"

#include "VulkanCameraBuffers.h"

#include "Core/ThreadSyncManager.h"
#include "Core/Logger.h"

namespace sh::render
{
	MaterialData::MaterialData(MaterialData&& other) noexcept :
		context(other.context),
		shader(other.shader),
		perPassData(std::move(other.perPassData)),
		bPerObject(other.bPerObject),
		bClearDirty(other.bClearDirty), bCreateDirty(other.bCreateDirty),
		syncDatas(std::move(other.syncDatas))
	{
		other.bClearDirty = false;
		other.bCreateDirty = false;
	}
	SH_RENDER_API void MaterialData::Create(const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		this->context = &context;
		this->shader = &shader;
		this->bPerObject = bPerObject;

		Clear();
		bCreateDirty = true;
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::Clear()
	{
		syncDatas.clear();
		bClearDirty = true;
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::Sync()
	{
		if (bClearDirty)
		{
			perPassData.clear();
			bClearDirty = false;
		}
		if (bCreateDirty && context != nullptr && core::IsValid(shader))
		{
			CreateBuffers(*this->context, *shader, bPerObject);
			bCreateDirty = false;
		}

		if (perPassData.empty())
		{
			syncDatas.clear();
			bDirty = false;
			return;
		}

		for (const auto& syncData : syncDatas)
		{
			if (std::holds_alternative<SyncData::BufferSyncData>(syncData.data)) // 버퍼
			{
				const SyncData::BufferSyncData& bufferData = std::get<0>(syncData.data);
				SetUniformDataAtSync(bufferData);
			}
			else // 텍스쳐
			{
				auto& uniformBufferData = std::get<1>(syncData.data);
				SetTextureDataAtSync(uniformBufferData);
			}
		}
		syncDatas.clear();

		bDirty = false;
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Usage usage, uint32_t binding, const void* data, std::size_t dataSize)
	{
		SyncData::BufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(usage);
		syncData.binding = binding;
		syncData.data.resize(dataSize);
		std::memcpy(syncData.data.data(), data, syncData.data.size());

		syncDatas.push_back(SyncData{ std::move(syncData) });
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Usage usage, uint32_t binding, std::vector<uint8_t> data)
	{
		SyncData::BufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(usage);
		syncData.binding = binding;
		syncData.data = std::move(data);

		syncDatas.push_back(SyncData{ std::move(syncData) });
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Usage usage, uint32_t binding, const Texture& tex)
	{
		SyncData::UniformBufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(usage);
		syncData.binding = binding;
		syncData.tex = &tex;

		syncDatas.push_back(SyncData{ std::move(syncData) });

		SyncDirty();
	}
	SH_RENDER_API auto MaterialData::GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Usage usage, uint32_t binding) const -> IBuffer*
	{
		const PassData* passData = GetMaterialPassData(shaderPass);
		if (passData == nullptr)
			return nullptr;

		uint32_t set = static_cast<uint32_t>(usage);
		auto itSet = passData->buffers.find(set);
		if (itSet == passData->buffers.end())
			return nullptr;

		const auto& buffers = itSet->second;
		if (binding >= buffers.size())
			return nullptr;

		return buffers[binding] ? buffers[binding].get() : nullptr;
	}
	SH_RENDER_API auto MaterialData::GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Usage usage) const -> IUniformBuffer*
	{
		const PassData* passData = GetMaterialPassData(shaderPass);
		if (passData == nullptr)
			return nullptr;

		uint32_t set = static_cast<uint32_t>(usage);
		auto it = passData->uniformBuffer.find(set);
		if (it == passData->uniformBuffer.end())
			return nullptr;

		return it->second.get();
	}

	SH_RENDER_API void MaterialData::SyncDirty()
	{
		if (bDirty)
			return;
		bDirty = true;
		core::ThreadSyncManager::PushSyncable(*this);
	}

	void MaterialData::CreateBuffers(const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		std::initializer_list<UniformStructLayout::Usage> usages = { UniformStructLayout::Usage::Camera, UniformStructLayout::Usage::Material };
		if (bPerObject)
			usages = { UniformStructLayout::Usage::Object };

		for (auto& [passName, shaderPasses] : shader.GetAllShaderPass())
		{
			for (ShaderPass& shaderPass : shaderPasses)
			{
				PassData passData{};
				uint32_t maxSet = 0;
				std::vector<uint32_t> sets;
				// 버퍼 (텍스쳐외 모든 GPU에 저장할 데이터)
				for (const std::vector<UniformStructLayout>* layouts : { &shaderPass.GetVertexUniforms(), &shaderPass.GetFragmentUniforms() })
				{
					for (const UniformStructLayout& uniformLayout : *layouts)
					{
						if (uniformLayout.IsPushConstant())
							continue;
						if (std::find(usages.begin(), usages.end(), uniformLayout.usage) == usages.end())
							continue;

						const uint32_t set = static_cast<uint32_t>(uniformLayout.usage);
						maxSet = (maxSet < set) ? set : maxSet;
						if (std::find(sets.begin(), sets.end(), set) == sets.end())
							sets.push_back(set);

						std::vector<std::unique_ptr<IBuffer>>& bindingBuffers = passData.buffers[set];
						if (bindingBuffers.size() <= uniformLayout.binding)
							bindingBuffers.resize(uniformLayout.binding + 1);

						if (set == 0) // 카메라 데이터는 다른 곳에서 관리한다.
							continue;

						BufferFactory::CreateInfo info{};
						info.size = uniformLayout.GetSize();
						info.bDynamic = (uniformLayout.GetKind() == UniformStructLayout::Kind::Storage);
						if (info.bDynamic && info.size == 0)
							info.size = 1; // 실제 데이터 업로드 시 Resize됨
						bindingBuffers[uniformLayout.binding] = BufferFactory::Create(context, info);
					}
				}
				for (const UniformStructLayout& layout : shaderPass.GetSamplerUniforms())
				{
					uint32_t set = static_cast<uint32_t>(layout.usage);
					maxSet = (maxSet < set) ? set : maxSet;
					if (std::find(sets.begin(), sets.end(), set) == sets.end())
						sets.push_back(set);

					std::vector<std::unique_ptr<IBuffer>>& bindingBuffers = passData.buffers[set];
					if (bindingBuffers.size() <= layout.binding)
						bindingBuffers.resize(layout.binding + 1);
				}
				// 유니폼 버퍼 (GPU로 데이터 전송 역할)
				for (uint32_t set : sets)
				{
					passData.uniformBuffer[set] = BufferFactory::CreateUniformBuffer(context, shaderPass, static_cast<UniformStructLayout::Usage>(set));
					auto it = passData.buffers.find(set);
					if (it == passData.buffers.end())
						continue;
					const std::vector<std::unique_ptr<IBuffer>>& bufferVec = it->second;
					for (uint32_t binding = 0; binding < bufferVec.size(); ++binding)
					{
						if (set != 0) // 카메라 데이터는 다른 곳에서 관리한다.
						{
							if (bufferVec[binding] == nullptr)
								continue;
							passData.uniformBuffer[set]->Link(binding, *bufferVec[binding].get());
						}
						else
						{
							assert(context.GetRenderAPIType() == RenderAPI::Vulkan); // 언젠가 API 추가되면 빼기
							// 카메라 데이터
							if (context.GetRenderAPIType() == RenderAPI::Vulkan)
							{
								passData.uniformBuffer[set]->Link(binding, vk::VulkanCameraBuffers::GetInstance()->GetCameraBuffer(), 128);
							}
						}
					}
				}

				passData.pass = &shaderPass;
				perPassData.insert_or_assign(&shaderPass, std::move(passData));
			}
		}
	}
	auto sh::render::MaterialData::GetMaterialPassData(const ShaderPass& shaderPass) const -> const MaterialData::PassData*
	{
		auto it = perPassData.find(&shaderPass);
		if (it == perPassData.end())
			return nullptr;
		return &it->second;
	}
	void MaterialData::SetUniformDataAtSync(const SyncData::BufferSyncData& bufferSyncData)
	{
		if (!core::IsValid(bufferSyncData.pass))
			return;

		const PassData* passData = GetMaterialPassData(*bufferSyncData.pass);
		if (passData == nullptr)
			return;

		const uint32_t set = bufferSyncData.set;
		auto itSet = passData->buffers.find(set);
		if (itSet == passData->buffers.end())
			return;

		const uint32_t binding = bufferSyncData.binding;
		const std::vector<std::unique_ptr<IBuffer>>& bufferVec = itSet->second;
		if (bufferSyncData.binding >= bufferVec.size() || bufferVec[bufferSyncData.binding] == nullptr)
			return;

		if (bufferVec[binding]->GetSize() != bufferSyncData.data.size())
		{
			for (const std::vector<UniformStructLayout>* layouts : { &bufferSyncData.pass->GetVertexUniforms(), &bufferSyncData.pass->GetFragmentUniforms() })
			{
				for (const UniformStructLayout& layout : *layouts)
				{
					if (static_cast<uint32_t>(layout.usage) != set || layout.binding != binding || layout.GetKind() != UniformStructLayout::Kind::Storage)
						continue;
					bufferVec[binding]->Resize(bufferSyncData.data.size());
					passData->uniformBuffer.at(set)->Link(binding, *bufferVec[binding].get());
					bufferVec[binding]->SetData(bufferSyncData.data.data());
					return;
				}
			}
			SH_ERROR_FORMAT("Buffer size is different! expected: {}, current: {}", bufferVec[binding]->GetSize(), bufferSyncData.data.size());
			return;
		}

		bufferVec[binding]->SetData(bufferSyncData.data.data());
	}
	void MaterialData::SetTextureDataAtSync(const SyncData::UniformBufferSyncData& uniformBufferSyncData)
	{
		const ShaderPass* shaderPass = uniformBufferSyncData.pass;
		if (!core::IsValid(shaderPass) || !core::IsValid(uniformBufferSyncData.tex))
			return;

		const PassData* passData = GetMaterialPassData(*shaderPass);
		if (passData == nullptr)
			return;

		const uint32_t set = uniformBufferSyncData.set;
		const uint32_t binding = uniformBufferSyncData.binding;

		bool bFind = false;
		for (auto& layout : shaderPass->GetSamplerUniforms())
		{
			if (static_cast<uint32_t>(layout.usage) == set && layout.binding == binding)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
			return;

		auto itUb = passData->uniformBuffer.find(set);
		if (itUb == passData->uniformBuffer.end() || itUb->second == nullptr)
			return;

		IUniformBuffer& uniformBuffer = *itUb->second.get();
		uniformBuffer.Link(binding, *uniformBufferSyncData.tex);
	}
}//namespace