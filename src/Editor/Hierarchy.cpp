﻿#include "Game/PCH.h"
#include "Hierarchy.h"

#include "Game/World.h"
#include "Game/GameObject.h"

namespace sh::editor
{
	SH_EDITOR_API Hierarchy::Hierarchy(game::ImGUImpl& imgui, game::World& world) :
		UI(imgui),
		world(world),
		selected(0), selectedObj(nullptr),
		isDocking(false)
	{
	}

	void Hierarchy::DrawGameObjectHierarchy(game::GameObject* obj, core::SHashSet<game::GameObject*>& drawSet)
	{
		if (drawSet.find(obj) != drawSet.end())
			return;

		drawSet.insert(obj);

		bool isSelected = selectedObj == obj;
		bool nodeOpen = false;
		bool hasChildren = !obj->transform->GetChildren().empty();

		if (hasChildren) 
		{
			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow | (isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->name.c_str());
		}
		else {
			nodeOpen = ImGui::TreeNodeEx((void*)obj,
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf |
				ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_NoTreePushOnOpen |
				(isSelected ? ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected : 0),
				"%s", obj->name.c_str());
		}

		if (ImGui::IsItemClicked())
			selectedObj = obj;

		// 드래그 되는 대상의 시점
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_::ImGuiDragDropFlags_None))
		{
			ImGui::SetDragDropPayload("GameObject", &obj, sizeof(game::GameObject*));
			ImGui::Text("%s", obj->name.c_str());
			ImGui::EndDragDropSource();
		}
		// 드래그 받는 대상의 시점
		if (ImGui::BeginDragDropTarget()) 
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) 
			{
				IM_ASSERT(payload->DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *reinterpret_cast<game::GameObject**>(payload->Data);

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


		if (nodeOpen && hasChildren) {
			for (auto child : obj->transform->GetChildren())
				DrawGameObjectHierarchy(child->gameObject, drawSet);
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

		for (auto& obj : world.gameObjects)
		{
			if (obj->transform->GetParent() == nullptr)
				DrawGameObjectHierarchy(obj, drawSet);
		}

		// 빈 공간 드래그 시 부모 제거
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::InvisibleButton("HierarchyEmptySpace", ImGui::GetContentRegionAvail());
		if (ImGui::BeginDragDropTarget()) // 드래그 받는 대상의 시점
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) 
			{
				IM_ASSERT(payload->DataSize == sizeof(game::GameObject*));
				game::GameObject* draggedObj = *(game::GameObject**)payload->Data;

				if (draggedObj->transform->GetParent() != nullptr)
					draggedObj->transform->SetParent(nullptr);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}

	SH_EDITOR_API bool Hierarchy::IsDocking() const
	{
		return isDocking;
	}
	SH_EDITOR_API auto Hierarchy::GetSelected() const -> game::GameObject*
	{
		return selectedObj;
	}
}
