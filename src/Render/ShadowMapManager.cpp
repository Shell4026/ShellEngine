#include "ShadowMapManager.h"
#include "RenderTexture.h"
#include "RenderData.h"
#include "Renderer.h"
#include "ShelfPacker.h"
#include "Formats.hpp"
#include "IRenderContext.h"

#include "Core/Name.h"
#include "Core/SObject.h"
#include "Core/GarbageCollection.h"
#include "Core/Logger.h"

#include <algorithm>

namespace sh::render
{
	ShadowMapManager::ShadowMapManager() = default;
	ShadowMapManager::~ShadowMapManager()
	{
		Clear();
	}

	SH_RENDER_API void ShadowMapManager::Init(IRenderContext& ctx, uint32_t atlasSize)
	{
		this->ctx = &ctx;
		this->atlasSize = atlasSize;
		packer = std::make_unique<ShelfPacker>(static_cast<int>(atlasSize), static_cast<int>(atlasSize), 0);

		renderData.tag = core::Name{ "Depth" };
		renderData.priority = 1000; // 다른 패스보다 먼저 실행되도록
	}

	SH_RENDER_API void ShadowMapManager::Clear()
	{
		if (atlas != nullptr)
		{
			core::GarbageCollection::GetInstance()->RemoveRootSet(atlas);
			atlas = nullptr;
		}
		casters.clear();
		casterSlots.clear();
		casterLightSpace.clear();
		renderData = RenderData{};
		packer.reset();
		ctx = nullptr;
	}

	void ShadowMapManager::EnsureAtlas()
	{
		if (atlas != nullptr)
			return;
		if (ctx == nullptr)
		{
			SH_ERROR("ShadowMapManager: context is not initialized");
			return;
		}
		RenderTargetLayout layout{};
		layout.format = TextureFormat::None;
		layout.depthFormat = TextureFormat::D32;
		layout.bUseMSAA = false;

		atlas = core::SObject::Create<RenderTexture>(layout);
		atlas->SetSize(atlasSize, atlasSize);
		atlas->Build(*ctx);
		atlas->SetName("ShadowAtlas");
		// 매니저는 SObject가 아니므로 GC가 회수하지 않도록 루트셋에 등록
		core::GarbageCollection::GetInstance()->SetRootSet(atlas);
	}

	SH_RENDER_API void ShadowMapManager::Register(IShadowCaster& caster)
	{
		if (std::find(casters.begin(), casters.end(), &caster) != casters.end())
			return;
		casters.push_back(&caster);
	}

	SH_RENDER_API void ShadowMapManager::Unregister(IShadowCaster& caster)
	{
		casters.erase(std::remove(casters.begin(), casters.end(), &caster), casters.end());
		casterSlots.erase(&caster);
		casterLightSpace.erase(&caster);
	}

	SH_RENDER_API void ShadowMapManager::Submit(Renderer& renderer)
	{
		if (casters.empty())
			return;

		EnsureAtlas();
		if (atlas == nullptr || packer == nullptr)
			return;

		packer->Reset();
		casterSlots.clear();
		casterLightSpace.clear();

		renderData.renderViewers.clear();
		renderData.target = atlas;
		renderData.renderViewers.reserve(casters.size());

		for (const IShadowCaster* caster : casters)
		{
			if (caster == nullptr)
				continue;

			const uint32_t res = caster->GetShadowMapResolution();
			if (res == 0)
				continue;

			int x = 0, y = 0;
			if (!packer->Alloc(static_cast<int>(res), static_cast<int>(res), x, y))
			{
				SH_ERROR_FORMAT("ShadowMapManager: out of atlas space (atlas {}, requested {})",
					atlasSize, res);
				continue;
			}

			Slot slot{};
			slot.uvOffset = glm::vec2{ static_cast<float>(x) / atlasSize, static_cast<float>(y) / atlasSize };
			slot.uvSize = glm::vec2{ static_cast<float>(res) / atlasSize, static_cast<float>(res) / atlasSize };
			slot.valid = true;

			const glm::mat4 view = caster->GetShadowViewMatrix();
			const glm::mat4 proj = caster->GetShadowProjMatrix();

			casterSlots.emplace(caster, slot);
			casterLightSpace.emplace(caster, proj * view);

			RenderViewer viewer{};
			viewer.viewMatrix = view;
			viewer.projMatrix = proj;
			viewer.pos = caster->GetShadowPos();
			viewer.to = caster->GetShadowLookAt();
			viewer.viewportRect = glm::uvec4{ atlasSize * slot.uvOffset.x, atlasSize * slot.uvOffset.y, atlasSize * slot.uvSize.x, atlasSize * slot.uvSize.y };
			viewer.viewportScissor = viewer.viewportRect;
			renderData.renderViewers.push_back(viewer);
		}

		if (renderData.renderViewers.empty())
			return;

		renderer.PushRenderData(renderData);
	}

	SH_RENDER_API auto ShadowMapManager::GetSlot(const IShadowCaster& caster) const -> Slot
	{
		auto it = casterSlots.find(&caster);
		if (it == casterSlots.end())
			return Slot{};
		return it->second;
	}

	SH_RENDER_API auto ShadowMapManager::GetLightSpaceMatrix(const IShadowCaster& caster) const -> glm::mat4
	{
		auto it = casterLightSpace.find(&caster);
		if (it == casterLightSpace.end())
			return glm::mat4{ 1.f };
		return it->second;
	}
}//namespace
