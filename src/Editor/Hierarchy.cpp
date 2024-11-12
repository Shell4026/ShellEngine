#include "Game/PCH.h"
#include "Hierarchy.h"
#include "EditorWorld.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	SH_EDITOR_API Hierarchy::Hierarchy(game::ImGUImpl& imgui, EditorWorld& world) :
		UI(imgui),
		world(world),
		isDocking(false)
	{
		onGameObjectAddedListener.SetCallback([&](game::GameObject* obj)
			{
				objList.push_back(obj);
			}
		);
		world.onGameObjectAdded.Register(onGameObjectAddedListener);

		for (auto obj : world.gameObjects)
		{
			objList.push_back(obj);
		}
	}

	void Hierarchy::DrawInvisibleSpace(game::GameObject* obj)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
		ImGui::InvisibleButton(("ReorderDrop" + obj->GetName()).c_str(), ImVec2(-1, 1));

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

	void Hierarchy::DrawGameObjectHierarchy(game::GameObject* obj, core::SHashSet<game::GameObject*>& drawSet)
	{
		bool isSelected = world.GetSelectedObject() == obj;
		bool nodeOpen = false;
		bool hasChildren = !obj->transform->GetChildren().empty();

		DrawInvisibleSpace(obj);
		if (hasChildren) 
		{
			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow | (isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->GetName().c_str());
		}
		else {
			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf |
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_NoTreePushOnOpen |
				(isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->GetName().c_str());
		}
		// 객체 우클릭
		if (ImGui::BeginPopupContextItem((std::string{ "RightClickPopupGameObject" } + obj->GetName()).c_str()))
		{
			if (ImGui::Selectable("Delete"))
			{
				if (obj == world.GetSelectedObject())
					world.SetSelectedObject(nullptr);
				world.DestroyGameObject(*obj);
			}
			ImGui::EndPopup();
		}
		// 객체 선택
		if (ImGui::IsItemHovered())
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
			{
				world.SetSelectedObject(obj);
			}
		}

		// 드래그 되는 대상의 시점
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("GameObject", &obj, sizeof(game::GameObject*));
			ImGui::Text("%s", obj->GetName().c_str());
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

	SH_EDITOR_API void Hierarchy::Update()
	{

	}
	SH_EDITOR_API void Hierarchy::Render()
	{
		ImGuiWindowFlags style =
			//ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Hierarchy", nullptr, style);
		isDocking = ImGui::IsWindowDocked();

		core::SHashSet<game::GameObject*> drawSet{};

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
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) 
			{
				IM_ASSERT(payload->DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *(game::GameObject**)payload->Data;

				if (draggedObj->transform->GetParent() != nullptr)
					draggedObj->transform->SetParent(nullptr);

				objList.erase(std::find(objList.begin(), objList.end(), draggedObj));
				objList.push_back(draggedObj);
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

	SH_EDITOR_API bool Hierarchy::IsDocking() const
	{
		return isDocking;
	}
}
