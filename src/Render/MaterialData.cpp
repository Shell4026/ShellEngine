#include "MaterialData.h"
#include "Shader.h"
#include "ShaderPass.h"
#include "BufferFactory.h"
#include "IRenderContext.h"

#include "VulkanCameraBuffers.h"

#include "Core/ThreadSyncManager.h"

namespace sh::render
{
	void MaterialData::CreateBuffers(core::ThreadType thread, const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		auto usages = { UniformStructLayout::Type::Camera, UniformStructLayout::Type::Material };
		if (bPerObject)
			usages = { UniformStructLayout::Type::Object };

		uint32_t thr = static_cast<uint32_t>(thread);

		for (auto& [passName, shaderPasses] : shader.GetAllShaderPass())
		{
			for (auto& shaderPass : shaderPasses)
			{
				core::SyncArray<PassData> passData{};
				uint32_t maxSet = 0;
				core::SVector<uint32_t> sets;
				// 유니폼 데이터 버퍼
				auto layouts = { &shaderPass->GetVertexUniforms(), &shaderPass->GetFragmentUniforms() };
				for (auto layout : layouts)
				{
					for (auto& uniformLayout : *layout)
					{
						if (uniformLayout.bConstant)
							continue;
						if (std::find(usages.begin(), usages.end(), uniformLayout.type) == usages.end())
							continue;

						uint32_t set = static_cast<uint32_t>(uniformLayout.type);
						maxSet = (maxSet < set) ? set : maxSet;
						if (std::find(sets.begin(), sets.end(), set) == sets.end())
							sets.push_back(set);

						if (passData[thr].uniformData.size() <= set)
							passData[thr].uniformData.resize(set + 1);
						if (passData[thr].uniformData[set].size() <= uniformLayout.binding)
							passData[thr].uniformData[set].resize(uniformLayout.binding + 1);

						if (set == 0) // 카메라 데이터는 다른 곳에서 관리한다.
							continue;
						passData[thr].uniformData[set][uniformLayout.binding] = BufferFactory::Create(context, uniformLayout.GetSize());
					}
				}
				// 유니폼 버퍼
				passData[thr].uniformBuffer.resize(maxSet + 1);
				for (auto set : sets)
				{
					passData[thr].uniformBuffer[set] = BufferFactory::CreateUniformBuffer(context, *shaderPass, static_cast<UniformStructLayout::Type>(set));
					for (uint32_t binding = 0; binding < passData[thr].uniformData[set].size(); ++binding)
					{
						if (set != 0)
						{
							auto& buffer = passData[thr].uniformData[set][binding];
							if (buffer == nullptr)
								continue;
							passData[thr].uniformBuffer[set]->Link(binding, *buffer.get());
						}
						else
						{
							if (context.GetRenderAPIType() == RenderAPI::Vulkan)
							{
								passData[thr].uniformBuffer[set]->Link(binding, vk::VulkanCameraBuffers::GetInstance()->GetCameraBuffer(), 128);
							}
						}
					}
				}

				passData[thr].pass = shaderPass.get();
				perPassData[thr].push_back(std::move(passData[thr]));
			}
		}
	}

	MaterialData::MaterialData(MaterialData&& other) noexcept :
		context(other.context),
		shader(other.shader),
		perPassData(std::move(other.perPassData)),
		bPerObject(other.bPerObject),
		bDirty(other.bDirty),
		bRecreate(other.bRecreate)
	{
		other.bDirty = false;
		other.bRecreate = false;
	}

	SH_RENDER_API void MaterialData::Create(const IRenderContext& context, const Shader& shader, bool bPerObject)
	{
		this->context = &context;
		this->shader = &shader;
		this->bPerObject = bPerObject;

		if (perPassData[core::ThreadType::Game].empty())
		{
			CreateBuffers(core::ThreadType::Game, context, shader, bPerObject);
			if (perPassData[core::ThreadType::Render].empty())
				CreateBuffers(core::ThreadType::Render, context, shader, bPerObject);
			else
				bRecreate = true;
		}
		else
		{
			perPassData[core::ThreadType::Game].clear();
			CreateBuffers(core::ThreadType::Game, context, shader, bPerObject);
			bRecreate = true;
		}
		if (bRecreate)
			SetDirty();
	}
	SH_RENDER_API void MaterialData::Clear()
	{
		perPassData[core::ThreadType::Game].clear();
		SetDirty();
	}
	SH_RENDER_API void MaterialData::SetDirty()
	{
		if (bDirty)
			return;
		bDirty = true;

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);
	}
	SH_RENDER_API void MaterialData::Sync()
	{
		if (!bDirty)
			return;

		if (perPassData[core::ThreadType::Game].empty()) // Clear
			perPassData[core::ThreadType::Render].clear();
		else
		{
			std::swap(perPassData[core::ThreadType::Game], perPassData[core::ThreadType::Render]);
			if (bRecreate)
			{
				Create(*context, *shader, bPerObject);
				bRecreate = false;
			}
		}

		bDirty = false;
	}
	SH_RENDER_API void MaterialData::SetUniformData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const void* data, core::ThreadType thread)
	{
		auto it = std::find_if(perPassData[thread].begin(), perPassData[thread].end(),
			[&](const MaterialData::PassData& passData)
			{
				return passData.pass == &shaderPass;
			}
		);
		if (it == perPassData[thread].end())
			return;

		const MaterialData::PassData& uniformData = *it;
		uint32_t set = static_cast<uint32_t>(type);

		assert(!uniformData.uniformData.empty());
		assert(!uniformData.uniformData[set].empty());
		assert(uniformData.uniformData[set][binding] != nullptr);
		auto& dataPtr = uniformData.uniformData[set][binding];
		if (dataPtr == nullptr)
			return;

		dataPtr->SetData(data);
		if (thread == core::ThreadType::Game)
			SetDirty();
	}
	SH_RENDER_API void MaterialData::SetTextureData(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, const Texture* tex)
	{
		if (!core::IsValid(tex))
			return;

		uint32_t set = static_cast<uint32_t>(type);

		auto it = std::find_if(perPassData[core::ThreadType::Game].begin(), perPassData[core::ThreadType::Game].end(),
			[&](const MaterialData::PassData& passData)
			{
				return passData.pass == &shaderPass;
			}
		);
		if (it == perPassData[core::ThreadType::Game].end())
			return;

		if (std::find_if(shaderPass.GetSamplerUniforms().begin(), shaderPass.GetSamplerUniforms().end(), 
			[&](const UniformStructLayout& layout)
			{
				return layout.type == type && layout.binding == binding;
			}
		) == shaderPass.GetSamplerUniforms().end())
		{
			return;
		}

		it->uniformBuffer[set]->Link(binding, *tex);

		SetDirty();
	}
	SH_RENDER_API auto MaterialData::GetShaderBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, uint32_t binding, core::ThreadType thr) const -> IBuffer*
	{
		auto it = std::find_if(perPassData[thr].begin(), perPassData[thr].end(), 
			[&](const MaterialData::PassData& passData)
			{
				return passData.pass == &shaderPass;
			}
		);
		if (it == perPassData[thr].end())
			return nullptr;

		uint32_t set = static_cast<uint32_t>(type);
		if (it->uniformData.size() <= set)
			return nullptr;

		auto& dataPtr = it->uniformData[set][binding];
		if (dataPtr == nullptr)
			return nullptr;
		else
			return dataPtr.get();

		return nullptr;
	}
	SH_RENDER_API auto MaterialData::GetUniformBuffer(const ShaderPass& shaderPass, UniformStructLayout::Type type, core::ThreadType thr) const -> IUniformBuffer*
	{
		auto it = std::find_if(perPassData[thr].begin(), perPassData[thr].end(), 
			[&](const MaterialData::PassData& passData)
			{
				return passData.pass == &shaderPass;
			}
		);
		if (it == perPassData[thr].end())
			return nullptr;
		if (it->uniformBuffer.size() <= static_cast<uint32_t>(type))
			return nullptr;

		return it->uniformBuffer[static_cast<uint32_t>(type)].get();
	}
}//namespace