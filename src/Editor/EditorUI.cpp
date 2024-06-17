#include "EditorUI.h"

#include "game/GameObject.h"
#include "game/World.h"

#include <cstring>

namespace sh::editor
{
	using namespace sh::game;

	EditorUI::EditorUI(World& world, const game::ImGUI& imgui) :
		world(world), imgui(imgui),

		hierarchyWidth(0), hierarchyHeight(0),
		selected(-1),
		bViewportDocking(false), bHierarchyDocking(false),
		bAddComponent(false)
	{
		ImGui::SetCurrentContext(imgui.GetContext());
	}

	void EditorUI::DrawViewport()
	{
		
		/*
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags style =
			//ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoNavFocus |
			//ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		float windowWidth = world.renderer.GetWindow().width;
		float windowHeight = world.renderer.GetWindow().height;

		ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_::ImGuiCond_Once);
		ImGui::SetNextWindowSize({ windowWidth, windowHeight }, ImGuiCond_::ImGuiCond_Once);

		ImGui::Begin("Viewport", nullptr, style);
		bViewportDocking = ImGui::IsWindowDocked();
		//ImGuiID dockspace_id = ImGui::GetID("MyDockSpace2");
		//ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		ImGui::End();
		*/
	}

	void EditorUI::DrawHierarchy()
	{

		ImGuiWindowFlags style =
			//ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Hierarchy", nullptr, style);
		bHierarchyDocking = ImGui::IsWindowDocked();
		for (int i = 0; i < world.gameObjects.size(); ++i)
		{
			auto& obj = world.gameObjects[i];
			if (ImGui::Selectable(obj->name.c_str(), selected == i))
			{
				selected = i;
			}
		}
		
		ImGui::End();
	}

	void EditorUI::DrawInspector()
	{
		ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		//float windowWidth = world.renderer.GetWindow().width;
		//float windowHeight = world.renderer.GetWindow().height;

		//ImGui::SetNextWindowPos(ImVec2{ windowWidth - 220, 20 }, ImGuiCond_Once);
		//ImGui::SetNextWindowSize(ImVec2{ 200, windowHeight - 200.f }, ImGuiCond_Once);

		ImGui::Begin("Inspector", nullptr, style);

		if (selected != -1)
		{
			if (selected > world.gameObjects.size() - 1)
			{
				selected = -1;
				ImGui::End();
				return;
			}
			auto obj = world.gameObjects[selected].get();

			char name[30] = "";
			std::strcpy(name, obj->name.c_str());

			ImGui::SetNextItemWidth(100);
			if (ImGui::InputText("Name", name, sizeof(name)))
			{
				//std::cout << name << '\n';
			}

			ImGui::Separator();

			ImGui::LabelText("##ComponentsLabel", "Components");
			for (auto& component : obj->GetComponents())
			{
				if (ImGui::CollapsingHeader(component->GetType().GetName().data()))
				{
					auto& props = component->GetType().GetProperties();
					for (auto& prop : props)
					{
						auto type = prop.second.GetTypeName();
						if (type == core::reflection::GetTypeName<glm::vec3>())
						{
							glm::vec3* parameter = prop.second.Get<glm::vec3>(component.get());
							ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
							ImGui::SetNextItemWidth(50);
							ImGui::InputFloat(("##" + prop.first + "x").c_str(), &parameter->x);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(50);
							ImGui::InputFloat(("##" + prop.first + "y").c_str(), &parameter->y);
							ImGui::SameLine();
							ImGui::SetNextItemWidth(50);
							ImGui::InputFloat(("##" + prop.first + "z").c_str(), &parameter->z);
						}
						else if (type == core::reflection::GetTypeName<float>())
						{
							float* parameter = prop.second.Get<float>(component.get());
							ImGui::InputFloat(prop.first.c_str(), parameter);
						}
					}
				}
			}
			
			ImGui::Separator();

			if (ImGui::Button("Add Component", { -FLT_MIN, 0.0f }))
				bAddComponent = !bAddComponent;
			if (bAddComponent)
			{
				static int current = -1;
				std::vector<const char*> items;
				auto& components = world.componentModule.GetComponents();
				for (auto& comp : components)
					items.push_back(comp.first.c_str());

				//ImGui::SetNextItemWidth(100);
				ImGui::SetNextItemWidth(-FLT_MIN);
				if (ImGui::ListBox("##Components", &current, items.data(), items.size()))
				{
					obj->AddComponent(components.at(items[current])->New());
					bAddComponent = false;
				}
				
				if (ImGui::Button("Close")) 
					bAddComponent = false;
			}
				//obj->SetName(name);
		}

		ImGui::End();
	}

	void EditorUI::DrawProject()
	{
		static ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Project", nullptr, style);

		ImGui::End();
	}

	void EditorUI::Update()
	{
		if (!imgui.IsInit())
			return;

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode;
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiID dockspaceId = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);

		static bool first = true;
		if (first)
		{
			first = false;
			ImGui::DockBuilderRemoveNode(dockspaceId); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspaceId, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

			auto dockDown = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.25f, nullptr, &dockspaceId);
			auto dockLeft = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.2f, nullptr, &dockspaceId);
			auto dockRight = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.25f, nullptr, &dockspaceId);
			ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
			ImGui::DockBuilderDockWindow("Inspector", dockRight); 
			ImGui::DockBuilderDockWindow("Project", dockDown);
			ImGui::DockBuilderFinish(dockspaceId);
		}

		ImGui::End();
		//DrawViewport();
		DrawHierarchy();
		DrawInspector();
		DrawProject();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{

				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
}