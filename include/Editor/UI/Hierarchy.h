#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"
#include "Core/EventSubscriber.h"

#include "Game/WorldEvents.hpp"

#include <list>
#include <unordered_map>
namespace sh::game
{
	class GameObject;
	class World;
}
namespace sh::editor
{
	class EditorWorld;

	class Hierarchy
	{
	public:
		SH_EDITOR_API Hierarchy(EditorWorld& world);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 빈 공간에 드래그 된 객체의 작동 방식을 설정한다.
		/// @brief 뒤늦게 추가 된 기능으로 덮어 씌워진다.
		/// @param dragItem 드래그 드롭 아이템 이름
		/// @param func 작동 함수
		SH_EDITOR_API void RegisterDragItemFunction(const std::string& dragItem, const std::function<void(const ImGuiPayload& payload)>& func);

		SH_EDITOR_API void AddOtherWorld(game::World& world);

		SH_EDITOR_API bool IsDocking() const;
	private:
		/// @brief 오브젝트 사이 빈공간
		void DrawInvisibleSpace(game::GameObject* obj);
		void DrawGameObjectHierarchy(game::GameObject* obj, std::unordered_set<game::GameObject*>& drawSet, bool bCanDrag = true);
		void CopyGameobject();
		void RenderHierarchy(core::SList<game::GameObject*>& objList, bool bCanDrag = true);
	public:
		static constexpr const char* name = "Hierarchy";
	private:
		EditorWorld& world;
		game::World* otherWorld = nullptr;

		core::SList<game::GameObject*> objList;
		core::SList<game::GameObject*> objListOther;

		core::EventSubscriber<game::events::GameObjectEvent> gameObjectEventSubscriber;
		core::EventSubscriber<game::events::GameObjectEvent> gameObjectEventSubscriberOther;

		std::vector<std::pair<std::string, std::function<void(const ImGuiPayload& payload)>>> dragFunc;

		bool isDocking;
		bool isFocus = false;
	};
}//namespace