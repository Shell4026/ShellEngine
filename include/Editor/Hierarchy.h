#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/SContainer.hpp"

#include "Game/ImGUImpl.h"

namespace sh::game
{
	class World;
	class GameObject;
}
namespace sh::editor
{
	class Hierarchy : public UI
	{
	private:
		game::World& world;

		int selected;
		game::GameObject* selectedObj;

		bool isDocking;
	public:
		static constexpr const char* name = "Hierarchy";
	private:
		void DrawGameObjectHierarchy(game::GameObject* obj, core::SHashSet<game::GameObject*>& drawSet);
	public:
		SH_EDITOR_API Hierarchy(game::ImGUImpl& imgui, game::World& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;

		SH_EDITOR_API bool IsDocking() const;
		SH_EDITOR_API auto GetSelected() const -> game::GameObject*;
	};
}//namespace