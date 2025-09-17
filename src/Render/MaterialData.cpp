#include "MaterialData.h"
#include "Shader.h"
#include "ShaderPass.h"
#include "BufferFactory.h"
#include "IRenderContext.h"

#include "VulkanCameraBuffers.h"

#include "Core/ThreadSyncManager.h"

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
			perPassData.clear();
		if (bCreateDirty && context != nullptr && core::IsValid(shader))
			CreateBuffers(*this->context, *shader, bPerObject);

		if (perPassData.empty())
		{
			syncDatas.clear();
			return;
		}

		for (const auto& syncData : syncDatas)
		{
			if (syncData.data.index() == 0) // 버퍼
			{
				auto& bufferData = std::get<0>(syncData.data);
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
		bClearDirty = false;
		bCreateDirty = false;
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const void* data, std::size_t dataSize)
	{
		SyncData::BufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(type);
		syncData.binding = binding;
		syncData.data.resize(dataSize);
		std::memcpy(syncData.data.data(), data, syncData.data.size());

		syncDatas.push_back(SyncData{ std::move(syncData) });
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, std::vector<uint8_t>&& data)
	{
		SyncData::BufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(type);
		syncData.binding = binding;
		syncData.data = std::move(data);

		syncDatas.push_back(SyncData{ std::move(syncData) });
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const Texture& tex)
	{
		SyncData::UniformBufferSyncData syncData{};
		syncData.pass = &shaderPass;
		syncData.set = static_cast<uint32_t>(type);
		syncData.binding = binding;
		syncData.tex = &tex;

		syncDatas.push_back(SyncData{ std::move(syncData) });

		SyncDirty();
	}
	SH_RENDER_API auto MaterialData::GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding) const -> IBuffer*
	{
		const PassData* passData = GetMaterialPassData(shaderPass);
		if (passData == nullptr)
			return nullptr;

		uint32_t set = static_cast<uint32_t>(type);
		auto itSet = passData->uniformData.find(set);
		if (itSet == passData->uniformData.end())
			return nullptr;

		const auto& buffers = itSet->second;
		if (binding >= buffers.size())
			return nullptr;

		return buffers[binding] ? buffers[binding].get() : nullptr;
	}
	SH_RENDER_API auto MaterialData::GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type) const -> IUniformBuffer*
	{
		const PassData* passData = GetMaterialPassData(shaderPass);
		if (passData == nullptr)
			return nullptr;

		uint32_t set = static_cast<uint32_t>(type);
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
		std::vector<UniformStructLayout::Type> usages = { UniformStructLayout::Type::Camera, UniformStructLayout::Type::Material };
		if (bPerObject)
			usages = { UniformStructLayout::Type::Object };

		for (auto& [passName, shaderPasses] : shader.GetAllShaderPass())
		{
			for (ShaderPass& shaderPass : shaderPasses)
			{
				PassData passData{};
				uint32_t maxSet = 0;
				std::vector<uint32_t> sets;
				// 유니폼 데이터 버퍼 (텍스쳐외 모든 GPU에 저장할 데이터)
				auto layoutsPerPass = { shaderPass.GetVertexUniforms(), shaderPass.GetFragmentUniforms() };
				for (const std::vector<UniformStructLayout>& layouts : layoutsPerPass)
				{
					for (const UniformStructLayout& uniformLayout : layouts)
					{
						if (uniformLayout.bConstant)
							continue;
						if (std::find(usages.begin(), usages.end(), uniformLayout.type) == usages.end())
							continue;

						uint32_t set = static_cast<uint32_t>(uniformLayout.type);
						maxSet = (maxSet < set) ? set : maxSet;
						if (std::find(sets.begin(), sets.end(), set) == sets.end())
							sets.push_back(set);

						auto& bindingBuffers = passData.uniformData[set];
						if (bindingBuffers.size() <= uniformLayout.binding)
							bindingBuffers.resize(uniformLayout.binding + 1);

						if (set == 0) // 카메라 데이터는 다른 곳에서 관리한다.
							continue;
						bindingBuffers[uniformLayout.binding] = BufferFactory::Create(context, uniformLayout.GetSize());
					}
				}
				for (const UniformStructLayout& layout : shaderPass.GetSamplerUniforms())
				{
					uint32_t set = static_cast<uint32_t>(layout.type);
					maxSet = (maxSet < set) ? set : maxSet;
					if (std::find(sets.begin(), sets.end(), set) == sets.end())
						sets.push_back(set);

					auto& bindingBuffers = passData.uniformData[set];
					if (bindingBuffers.size() <= layout.binding)
						bindingBuffers.resize(layout.binding + 1);
				}
				// 유니폼 버퍼 (GPU로 데이터 전송 역할)
				for (uint32_t set : sets)
				{
					passData.uniformBuffer[set] = BufferFactory::CreateUniformBuffer(context, shaderPass, static_cast<UniformStructLayout::Type>(set));
					auto it = passData.uniformData.find(set);
					if (it == passData.uniformData.end())
						continue;
					auto& bufferVec = it->second;
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
		auto itSet = passData->uniformData.find(set);
		if (itSet == passData->uniformData.end())
			return;

		const uint32_t binding = bufferSyncData.binding;
		auto& bufferVec = itSet->second;
		if (bufferSyncData.binding >= bufferVec.size() || bufferVec[bufferSyncData.binding] == nullptr)
			return;

		if (bufferVec[binding]->GetSize() != bufferSyncData.data.size())
		{
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
			if (static_cast<uint32_t>(layout.type) == set && layout.binding == binding)
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