#include "MaterialData.h"
#include "Shader.h"
#include "ShaderPass.h"
#include "BufferFactory.h"
#include "IRenderContext.h"

#include "VulkanCameraBuffers.h"

#include "Core/ThreadSyncManager.h"

namespace sh::render
{
	void MaterialData::CreateBuffers(const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		std::vector<UniformStructLayout::Type> usages = { UniformStructLayout::Type::Camera, UniformStructLayout::Type::Material };
		if (bPerObject)
			usages = { UniformStructLayout::Type::Object };

		for (auto& [passName, shaderPasses] : shader.GetAllShaderPass())
		{
			for (ShaderPass* shaderPass : shaderPasses)
			{
				PassData passData{};
				uint32_t maxSet = 0;
				std::vector<uint32_t> sets;
				// 유니폼 데이터 버퍼 (텍스쳐외 모든 GPU에 저장할 데이터)
				auto layouts = { shaderPass->GetVertexUniforms(), shaderPass->GetFragmentUniforms()};
				for (auto& layout : layouts)
				{
					for (auto& uniformLayout : layout)
					{
						if (uniformLayout.bConstant)
							continue;
						if (std::find(usages.begin(), usages.end(), uniformLayout.type) == usages.end())
							continue;

						uint32_t set = static_cast<uint32_t>(uniformLayout.type);
						maxSet = (maxSet < set) ? set : maxSet;
						if (std::find(sets.begin(), sets.end(), set) == sets.end())
							sets.push_back(set);

						if (passData.uniformData.size() <= set)
							passData.uniformData.resize(set + 1);
						if (passData.uniformData[set].size() <= uniformLayout.binding)
							passData.uniformData[set].resize(uniformLayout.binding + 1);

						if (set == 0) // 카메라 데이터는 다른 곳에서 관리한다.
							continue;
						passData.uniformData[set][uniformLayout.binding] = BufferFactory::Create(context, uniformLayout.GetSize());
					}
				}
				for (auto& layout : shaderPass->GetSamplerUniforms())
				{
					uint32_t set = static_cast<uint32_t>(layout.type);
					maxSet = (maxSet < set) ? set : maxSet;
					if (std::find(sets.begin(), sets.end(), set) == sets.end())
						sets.push_back(set);
					if (passData.uniformData.size() <= set)
						passData.uniformData.resize(set + 1);
				}
				// 유니폼 버퍼 (GPU로 데이터 전송 역할)
				passData.uniformBuffer.resize(maxSet + 1);
				for (auto set : sets)
				{
					passData.uniformBuffer[set] = BufferFactory::CreateUniformBuffer(context, *shaderPass, static_cast<UniformStructLayout::Type>(set));
					for (uint32_t binding = 0; binding < passData.uniformData[set].size(); ++binding)
					{
						if (set != 0) // 카메라 데이터는 다른 곳에서 관리한다.
						{
							if (passData.uniformData[set].size() <= binding)
								continue;
							auto& buffer = passData.uniformData[set][binding];
							if (buffer == nullptr)
								continue;
							passData.uniformBuffer[set]->Link(binding, *buffer.get());
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

				passData.pass = shaderPass;
				perPassData.push_back(std::move(passData));
			}
		}
	}
	auto sh::render::MaterialData::GetMaterialPassData(const ShaderPass& shaderPass) const -> const MaterialData::PassData*
	{
		const MaterialData::PassData* uniformData = nullptr;
		for (auto& passData : perPassData)
		{
			if (passData.pass == &shaderPass)
			{
				uniformData = &passData;
				break;
			}
		}
		return uniformData;
	}

	MaterialData::MaterialData(MaterialData&& other) noexcept :
		context(other.context),
		shader(other.shader),
		perPassData(std::move(other.perPassData)),
		bPerObject(other.bPerObject),
		bRecreate(other.bRecreate),
		syncDatas(std::move(other.syncDatas))
	{
		other.bRecreate = false;
	}

	SH_RENDER_API void MaterialData::Create(const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		this->context = &context;
		this->shader = &shader;
		this->bPerObject = bPerObject;

		CreateBuffers(context, shader, bPerObject);
	}
	SH_RENDER_API void MaterialData::Clear()
	{
		perPassData.clear();
		SyncDirty();
	}
	SH_RENDER_API void MaterialData::Sync()
	{
		for (const auto& syncData : syncDatas)
		{
			if (syncData.data.index() == 0)
			{
				auto& bufferData = std::get<0>(syncData.data);
				bufferData.bufferPtr->SetData(bufferData.data.data());
			}
			else
			{
				auto& uniformBufferData = std::get<1>(syncData.data);
				if (core::IsValid(uniformBufferData.tex))
					uniformBufferData.bufferPtr->Link(uniformBufferData.binding, *uniformBufferData.tex);
			}
		}
		syncDatas.clear();

		bDirty = false;
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const void* data)
	{
		const MaterialData::PassData* uniformData = GetMaterialPassData(shaderPass);
		if (uniformData == nullptr)
			return;

		const uint32_t set = static_cast<uint32_t>(type);

		assert(!uniformData->uniformData.empty());
		assert(!uniformData->uniformData[set].empty());
		assert(uniformData->uniformData[set][binding] != nullptr);
		auto& dataPtr = uniformData->uniformData[set][binding];
		if (dataPtr == nullptr)
			return;

		SyncData::BufferSyncData bufferSyncData{};
		bufferSyncData.bufferPtr = dataPtr.get();
		bufferSyncData.data.resize(dataPtr->GetSize());
		std::memcpy(bufferSyncData.data.data(), data, dataPtr->GetSize());

		SyncData syncData{ std::move(bufferSyncData) };

		syncDatas.push_back(std::move(syncData));

		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, std::vector<uint8_t>&& data)
	{
		const MaterialData::PassData* uniformData = GetMaterialPassData(shaderPass);
		if (uniformData == nullptr)
			return;

		const uint32_t set = static_cast<uint32_t>(type);

		assert(!uniformData->uniformData.empty());
		assert(!uniformData->uniformData[set].empty());
		assert(uniformData->uniformData[set][binding] != nullptr);
		auto& dataPtr = uniformData->uniformData[set][binding];
		if (dataPtr == nullptr)
			return;

		SyncData::BufferSyncData bufferSyncData{};
		bufferSyncData.data = std::move(data);
		bufferSyncData.bufferPtr = dataPtr.get();

		SyncData syncData{ std::move(bufferSyncData) };

		syncDatas.push_back(std::move(syncData));

		SyncDirty();
	}
	SH_RENDER_API void MaterialData::SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const Texture* tex)
	{
		if (!core::IsValid(tex))
			return;

		const MaterialData::PassData* uniformData = GetMaterialPassData(shaderPass);
		if (uniformData == nullptr)
			return;

		uint32_t set = static_cast<uint32_t>(type);

		bool bFind = false;
		for (auto& layout : shaderPass.GetSamplerUniforms())
		{
			if (layout.type == type && layout.binding == binding)
			{
				bFind = true;
				break;
			}
		}
		if (!bFind)
			return;

		SyncData::UniformBufferSyncData uniformSyncData{};
		uniformSyncData.bufferPtr = uniformData->uniformBuffer[set].get();
		uniformSyncData.binding = binding;
		uniformSyncData.tex = tex;

		SyncData syncData{ std::move(uniformSyncData) };

		syncDatas.push_back(std::move(syncData));

		SyncDirty();
	}
	SH_RENDER_API auto MaterialData::GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding) const -> IBuffer*
	{
		const MaterialData::PassData* uniformData = GetMaterialPassData(shaderPass);
		if (uniformData == nullptr)
			return nullptr;

		uint32_t set = static_cast<uint32_t>(type);
		if (uniformData->uniformData.size() <= set)
			return nullptr;

		auto& dataPtr = uniformData->uniformData[set][binding];
		if (dataPtr == nullptr)
			return nullptr;
		else
			return dataPtr.get();

		return nullptr;
	}
	SH_RENDER_API auto MaterialData::GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type) const -> IUniformBuffer*
	{
		const MaterialData::PassData* uniformData = GetMaterialPassData(shaderPass);
		if (uniformData == nullptr)
			return nullptr;

		if (uniformData->uniformBuffer.size() <= static_cast<uint32_t>(type))
			return nullptr;

		return uniformData->uniformBuffer[static_cast<uint32_t>(type)].get();
	}

	SH_RENDER_API void MaterialData::SyncDirty()
	{
		if (bDirty)
			return;
		bDirty = true;
		core::ThreadSyncManager::PushSyncable(*this);
	}
}//namespace