﻿#include "UI/Hierarchy.h"
#include "UI/CustomHierarchy.h"
#include "EditorWorld.h"

#include "Game/ImGUImpl.h"
#include "Game/GameObject.h"
#include "Game/Input.h"

namespace sh::editor
{
	Hierarchy::Hierarchy(EditorWorld& world) :
		world(world),
		isDocking(false)
	{
		gameObjectEventSubscriber.SetCallback(
			[&](const game::events::GameObjectEvent& event)
			{
				if (event.type == game::events::GameObjectEvent::Type::Added)
				{
					if (!event.gameObject.IsPendingKill())
						objList.push_back(&event.gameObject);
				}
			}
		);
		world.SubscribeEvent(gameObjectEventSubscriber);

		for (auto obj : world.GetGameObjects())
		{
			objList.push_back(obj);
		}

		RegisterDragItemFunction("GameObject", 
			[&](const ImGuiPayload& payload)
			{
				IM_ASSERT(payload.DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *(game::GameObject**)payload.Data;

				if (draggedObj->transform->GetParent() != nullptr)
					draggedObj->transform->SetParent(nullptr);

				objList.erase(std::find(objList.begin(), objList.end(), draggedObj));
				objList.push_back(draggedObj);
			}
		);
	}

	void Hierarchy::DrawInvisibleSpace(game::GameObject* obj)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		ImGui::InvisibleButton(("ReorderDrop" + obj->GetName().ToString()).c_str(), ImVec2(-1, 1));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject"))
			{
				IM_ASSERT(payload->DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *(game::GameObject**)payload->Data;

				// 순서 변경
				if (obj->transform->GetParent() == nullptr)
				{
					objList.erase(std::find(objList.begin(), objList.end(), draggedObj));
					auto it = std::find(objList.begin(), objList.end(), obj);
					if (it != objList.begin())
						objList.insert(it, draggedObj);
					else
						objList.push_front(draggedObj);
				}
				else
				{
					if (obj->transform->GetParent() == draggedObj->transform->GetParent())
					{
						auto objParent = obj->transform->GetParent();
						auto objIt = std::find(objParent->GetChildren().begin(), objParent->GetChildren().end(), obj->transform);
						auto draggedIt = std::find(objParent->GetChildren().begin(), objParent->GetChildren().end(), draggedObj->transform);
						uint32_t dif = draggedIt - objIt;
						for (std::size_t i = 0; i < dif; ++i)
						{
							objParent->ReorderChildAbove(draggedObj->transform);
						}
					}
					else
					{
						auto objParent = obj->transform->GetParent();
						draggedObj->transform->SetParent(objParent);
						auto objIt = std::find(objParent->GetChildren().begin(), objParent->GetChildren().end(), obj->transform);
						auto draggedIt = std::find(objParent->GetChildren().begin(), objParent->GetChildren().end(), draggedObj->transform);
						uint32_t dif = draggedIt - objIt;
						for (std::size_t i = 0; i < dif; ++i)
						{
							objParent->ReorderChildAbove(draggedObj->transform);
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	void Hierarchy::DrawGameObjectHierarchy(game::GameObject* obj, std::unordered_set<game::GameObject*>& drawSet)
	{
		bool isSelected = world.IsSelected(obj);
		bool nodeOpen = false;
		bool hasChildren = !obj->transform->GetChildren().empty();
		bool bActive = obj->activeSelf;

		DrawInvisibleSpace(obj);
		if (hasChildren) 
		{
			if (!bActive)
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow | (isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->GetName().ToString().c_str());

			if (!bActive)
				ImGui::PopStyleColor();
		}
		else {
			if (!bActive)
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf |
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_NoTreePushOnOpen |
				(isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->GetName().ToString().c_str());

			if (!bActive)
				ImGui::PopStyleColor();
		}
		// 객체 우클릭
		if (ImGui::BeginPopupContextItem((obj->GetUUID().ToString() + "RightClickPopup").c_str()))
		{
			if (ImGui::Selectable("Delete"))
			{
				world.DestroyGameObject(*static_cast<game::GameObject*>(obj));

				auto& selectedObjs = world.GetSelectedObjects();
				for (auto selectedObj : selectedObjs)
				{
					if (selectedObj == obj)
						continue;

					if (core::IsValid(selectedObj) && selectedObj->GetType() == game::GameObject::GetStaticType())
						world.DestroyGameObject(*static_cast<game::GameObject*>(selectedObj));
				}
			}
			ImGui::EndPopup();
		}
		// 객체 선택
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				if (!game::Input::GetKeyDown(game::Input::KeyCode::Shift))
					world.ClearSelectedObjects();
				world.AddSelectedObject(obj);
			}
		}

		// 드래그 되는 대상의 시점
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("GameObject", &obj, sizeof(game::GameObject*));
			ImGui::Text("%s", obj->GetName().ToString().c_str());
			ImGui::EndDragDropSource();
		}
		// 드래그 받는 대상의 시점
		if (ImGui::BeginDragDropTarget()) 
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) 
			{
				IM_ASSERT(payload->DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *reinterpret_cast<game::GameObject**>(payload->Data);

				// 순환 참조 체크, 부모 설정
				bool circulation = false;
				game::Transform* parent = obj->transform->GetParent();
				while (parent)
				{
					if (parent == draggedObj->transform)
					{
						circulation = true;
						break;
					}
					parent = parent->GetParent();
				}
				if(!circulation)
					draggedObj->transform->SetParent(obj->transform);
			}
			ImGui::EndDragDropTarget();
		}

		if (nodeOpen && hasChildren) 
		{
			for (auto child : obj->transform->GetChildren())
			{
				if (child->gameObject.hideInspector)
					continue;
				DrawGameObjectHierarchy(&child->gameObject, drawSet);
			}
			ImGui::TreePop();
		}
	}

	void Hierarchy::CopyGameobject()
	{
		auto& selectedObjs = world.GetSelectedObjects();
		for (auto selectedObj : selectedObjs)
		{
			if (!core::IsValid(selectedObj))
				continue;

			game::GameObject& gameObj = *static_cast<game::GameObject*>(selectedObj);

			gameObj.Clone();
		}
	}

	SH_EDITOR_API void Hierarchy::Update()
	{
		if (game::Input::GetKeyDown(game::Input::KeyCode::LCtrl) && game::Input::GetKeyPressed(game::Input::KeyCode::D))
			CopyGameobject();
	}
	SH_EDITOR_API void Hierarchy::Render()
	{
		ImGuiWindowFlags style =
			//ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		ImGui::SetNextWindowSize({ 200, 500 });
		ImGui::Begin("Hierarchy", nullptr, style);
		isDocking = ImGui::IsWindowDocked();
		isFocus = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();

		std::unordered_set<game::GameObject*> drawSet{};

		for (auto it = objList.begin(); it != objList.end();)
		{
			auto obj = *it;
			if (!core::IsValid(obj))
			{
				it = objList.erase(it);
				continue;
			}
			if (obj->hideInspector)
			{
				++it;
				continue;
			}
			if (obj->transform->GetParent() == nullptr)
				DrawGameObjectHierarchy(obj, drawSet);
			++it;
		}

		// 빈 공간 드래그 시 부모 제거
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::InvisibleButton("HierarchyEmptySpace", ImGui::GetContentRegionAvail());
		if (ImGui::BeginDragDropTarget())
		{
			const auto customHierarchyManager = CustomHierarchyManager::GetInstance();
			for (const auto& [stype, customHierarchy] : customHierarchyManager->map)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(std::string{ stype->type.name }.c_str()))
					customHierarchy->OnHierarchyDraged(world, *payload);
			}
			for (auto& [name, func] : dragFunc)
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject"))
					func(*payload);
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleVar();

		// 우클릭
		if (ImGui::BeginPopupContextItem("RightClickPopup"))
		{
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("GameObject"))
				{
					world.AddGameObject("EmptyObject");
				}
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	SH_EDITOR_API void Hierarchy::RegisterDragItemFunction(const std::string& dragItem, const std::function<void(const ImGuiPayload& payload)>& func)
	{
		for (auto& [name, _func] : dragFunc)
		{
			if (name == dragItem)
			{
				_func = func;
				return;
			}
		}
		dragFunc.push_back({ dragItem, func });
	}

	SH_EDITOR_API bool Hierarchy::IsDocking() const
	{
		return isDocking;
	}
}
