#pragma once
#include "Export.h"

#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"

#include <list>
#include <unordered_map>
namespace sh::game
{
	class GameObject;
}
namespace sh::editor
{
	class EditorWorld;

	class Hierarchy
	{
	private:
		EditorWorld& world;

		core::SList<game::GameObject*> objList;
		core::Observer<false, game::GameObject*>::Listener onGameObjectAddedListener;
		//core::Observer<game::GameObject*>::Listener onGameObjectRemovedListener;

		std::vector<std::pair<std::string, std::function<void(const ImGuiPayload& payload)>>> dragFunc;

		bool isDocking;
		bool isFocus = false;
	public:
		static constexpr const char* name = "Hierarchy";
	private:
		/// @brief 오브젝트 사이 빈공간
		void DrawInvisibleSpace(game::GameObject* obj);
		void DrawGameObjectHierarchy(game::GameObject* obj, std::unordered_set<game::GameObject*>& drawSet);
		void CopyGameobject();
	public:
		SH_EDITOR_API Hierarchy(EditorWorld& world);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		/// @brief 빈 공간에 드래그 된 객체의 작동 방식을 설정한다.
		/// @brief 뒤늦게 추가 된 기능으로 덮어 씌워진다.
		/// @param dragItem 드래그 드롭 아이템 이름
		/// @param func 작동 함수
		SH_EDITOR_API void RegisterDragItemFunction(const std::string& dragItem, const std::function<void(const ImGuiPayload& payload)>& func);

		SH_EDITOR_API bool IsDocking() const;
	};
}//namespace