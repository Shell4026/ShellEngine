#include "EditorUI.h"
#include "Project.h"

#include "game/GameObject.h"
#include "game/World.h"

#include "Render/VulkanImpl/VulkanTextureBuffer.h"
#include "Render/RenderTexture.h"

#include <cstring>

namespace sh::editor
{
	using namespace sh::game;

	EditorUI::EditorUI(World& world, game::ImGUImpl& imgui) :
		UI(imgui),
		world(world),
		viewport(imgui, world),
		hierarchy(imgui, world),

		hierarchyWidth(0), hierarchyHeight(0),
		explorer(imgui),

		bAddComponent(false), bOpenExplorer(false),
		bDirty(false)
	{
	}

	void EditorUI::SetDockNode()
	{
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

		dockspaceId = ImGui::GetID("MyDockSpace");
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
			ImGui::DockBuilderDockWindow(Hierarchy::name, dockLeft);
			ImGui::DockBuilderDockWindow("Inspector", dockRight);
			ImGui::DockBuilderDockWindow("Project", dockDown);
			ImGui::DockBuilderDockWindow(this->viewport.name, dockspaceId);
			ImGui::DockBuilderFinish(dockspaceId);
		}
		ImGui::End();
	}

	void EditorUI::DrawInspector()
	{
		ImGuiWindowFlags style =
			ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse;

		ImGui::Begin("Inspector", nullptr, style);

		if (auto obj = hierarchy.GetSelected(); core::IsValid(obj))
		{
			char name[30] = "";
			std::strcpy(name, obj->name.c_str());

			ImGui::SetNextItemWidth(100);
			if (ImGui::InputText("Name", name, sizeof(name)))
			{
				//std::cout << name << '\n';
			}
			ImGui::GetDrawData();

			ImGui::Separator();

			ImGui::LabelText("##ComponentsLabel", "Components");
			int idx = 0;
			for (auto& component : obj->GetComponents())
			{
				if (ImGui::CollapsingHeader((component->GetType().GetName().data() + ("##" + std::to_string(idx))).data()))
				{
					auto currentType = &component->GetType();
					do
					{
						auto& props = currentType->GetProperties();
						for (auto& prop : props)
						{
							if (prop.second.bVisible == false)
								continue;
							auto type = prop.second.GetTypeName();
							bool constant = prop.second.isConst;
							auto inputFlag = constant ? ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_::ImGuiInputTextFlags_None;
							if (type == core::reflection::GetTypeName<glm::vec3>())
							{
								glm::vec3* parameter = prop.second.Get<glm::vec3>(component);
								float v[3] = { parameter->x, parameter->y, parameter->z };
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
								{
									ImGui::InputFloat3(("##" + prop.first + std::to_string(idx)).c_str(), v, "%.3f", ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								}
								else
								{
									if (ImGui::InputFloat3(("##" + prop.first + std::to_string(idx)).c_str(), v))
									{
										parameter->x = v[0];
										parameter->y = v[1];
										parameter->z = v[2];
										component->OnPropertyChanged(prop.second);
									}
								}
							}
							else if (type == core::reflection::GetTypeName<float>())
							{
								float* parameter = prop.second.Get<float>(component);
								if (ImGui::InputFloat(("##input_" + prop.first + std::to_string(idx)).c_str(), parameter))
									component->OnPropertyChanged(prop.second);
							}
							else if (type == core::reflection::GetTypeName<int>())
							{
								int* parameter = prop.second.Get<int>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
									ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter, 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								else
									if (ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter))
									{
										component->OnPropertyChanged(prop.second);
									}
							}
							else if (type == core::reflection::GetTypeName<uint32_t>())
							{
								uint32_t* parameter = prop.second.Get<uint32_t>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if (prop.second.isConst)
									ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter), 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
								else
									if (ImGui::InputInt(("##Input_" + prop.first + std::to_string(idx)).c_str(), reinterpret_cast<int*>(parameter)))
										component->OnPropertyChanged(prop.second);
							}
							else if (type == core::reflection::GetTypeName<std::string>())
							{
								std::string* parameter = prop.second.Get<std::string>(component);
								ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
								if(ImGui::InputText(("##Input_" + prop.first + std::to_string(idx)).c_str(), parameter, inputFlag))
									component->OnPropertyChanged(prop.second);
							}
						}
						currentType = const_cast<core::reflection::TypeInfo*>(currentType->GetSuper());
					} 
					while (currentType);
				}
				++idx;
			}//for auto& component
			
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
					std::string name = items[current];
					obj->AddComponent(components.at(name)->Create());
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
		viewport.Update();
		Render();
	}

	void EditorUI::Render()
	{
		if (!imgui.IsInit())
			return;

		SetDockNode();
		hierarchy.Render();
		DrawInspector();
		DrawProject();
		viewport.Render();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("OpenProject", "Ctrl+O"))
				{
					bOpenExplorer = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (bOpenExplorer)
			explorer.Update();

		imgui.SetDirty();
	}

	auto EditorUI::GetViewport() -> Viewport&
	{
		return viewport;
	}

	void EditorUI::Clean()
	{
		viewport.Clean();
	}
}