#pragma once

#include "MeshRenderer.h"
#include "PickingCamera.h"

#include "Core/Singleton.hpp"

namespace sh::game
{
	class PickingRenderer;

	/// @brief PickingRenderer 객체의 아이디를 전역적으로 관리하는 클래스
	class PickingIdManager
	{
	private:
		SH_GAME_API static inline uint32_t nextId = 1;
		SH_GAME_API static inline core::SHashMap<uint32_t, PickingRenderer*> ids{};	
		SH_GAME_API static inline std::queue<uint32_t> emptyId{};
	public:
		SH_GAME_API static auto AssignId(PickingRenderer* renderer) -> uint32_t;
		SH_GAME_API static auto Get(uint32_t id) -> PickingRenderer*;
		SH_GAME_API static void Erase(uint32_t id);
	};

	class PickingRenderer : public MeshRenderer
	{
		COMPONENT(PickingRenderer)
	private:
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
		PROPERTY(camera)
		PickingCamera* camera = nullptr;
		PROPERTY(id)
		uint32_t id = 0;
	public:
		SH_GAME_API PickingRenderer(GameObject& owner);
		SH_GAME_API ~PickingRenderer();

		SH_GAME_API void Awake() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SetCamera(PickingCamera& camera);

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	};
}//namespace