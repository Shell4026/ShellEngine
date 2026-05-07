#include "Game/Component/Render/LightBase.h"
#include "Game/World.h"

namespace sh::game
{
	LightBase::LightBase(GameObject& owner) :
		Component(owner)
	{
		canPlayInEditor = true;
	}
	SH_GAME_API void LightBase::Awake()
	{
		Super::Awake();
		if (bCastShadow)
			RegisterToShadowManager();
		world.GetLightOctree().Insert(*this);
	}
	SH_GAME_API void LightBase::OnDestroy()
	{
		UnregisterFromShadowManager();
		world.GetLightOctree().Erase(*this);
		Super::OnDestroy();
	}
	SH_GAME_API void LightBase::OnEnable()
	{
		if (bCastShadow)
			RegisterToShadowManager();
		world.GetLightOctree().Insert(*this);
	}
	SH_GAME_API void LightBase::OnDisable()
	{
		UnregisterFromShadowManager();
		world.GetLightOctree().Erase(*this);
	}
	SH_GAME_API void LightBase::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == "bCastShadow")
		{
			if (bCastShadow)
				RegisterToShadowManager();
			else
				UnregisterFromShadowManager();
		}
	}
	SH_GAME_API void LightBase::SetCastShadow(bool _bCastShadow)
	{
		if (bCastShadow == _bCastShadow)
			return;
		bCastShadow = _bCastShadow;
		if (bCastShadow)
			RegisterToShadowManager();
		else
			UnregisterFromShadowManager();
	}
	SH_GAME_API auto LightBase::GetShadowMap() const -> render::RenderTexture*
	{
		if (!bCastShadow)
			return nullptr;
		return world.GetShadowMapManager().GetAtlas();
	}
	SH_GAME_API auto LightBase::GetShadowSlot() const -> render::ShadowMapManager::Slot
	{
		if (!bCastShadow)
			return render::ShadowMapManager::Slot{};
		return world.GetShadowMapManager().GetSlot(*this);
	}
	SH_GAME_API void LightBase::UpdateLightOctree()
	{
		world.GetLightOctree().Erase(*this);
		world.GetLightOctree().Insert(*this);
	}
	SH_GAME_API void LightBase::RegisterToShadowManager()
	{
		if (bRegistered)
			return;
		world.GetShadowMapManager().Register(*this);
		bRegistered = true;
	}

	SH_GAME_API void LightBase::UnregisterFromShadowManager()
	{
		if (!bRegistered)
			return;
		world.GetShadowMapManager().Unregister(*this);
		bRegistered = false;
	}
}//namespace