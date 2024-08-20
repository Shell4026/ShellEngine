﻿#include "EditorUI.h"
#include "Project.h"

#include "game/GameObject.h"
#include "game/World.h"

#include "Render/VulkanTextureBuffer.h"
#include "Render/RenderTexture.h"

#include <cstring>

namespace sh::editor
{
	using namespace sh::game;

	EditorUI::EditorUI(World& world, const game::ImGUI& imgui) :
		UI(imgui),
		world(world),

		hierarchyWidth(0), hierarchyHeight(0),
		selected(-1),
		explorer(imgui),
		viewportWidthLast(100.f), viewportHeightLast(100.f),
		viewportDescSet(nullptr),

		bViewportDocking(false), bHierarchyDocking(false),
		bAddComponent(false), bOpenExplorer(false), bChangedViewportSize(false)
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
			ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
			ImGui::DockBuilderDockWindow("Inspector", dockRight);
			ImGui::DockBuilderDockWindow("Project", dockDown);
			ImGui::DockBuilderDockWindow("Viewport", dockspaceId);
			ImGui::DockBuilderFinish(dockspaceId);
		}
		ImGui::End();
	}

	void EditorUI::DrawViewport()
	{
		ImGui::Begin("Viewport");
		float width = ImGui::GetContentRegionAvail().x;
		float height = ImGui::GetContentRegionAvail().y;
		if (viewportWidthLast != width || viewportHeightLast != height)
		{
			viewportWidthLast = width;
			viewportHeightLast = height;
			bChangedViewportSize = true;
		}
		ImGui::Image((ImTextureID)viewportDescSet, { width, height });
		ImGui::End();
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
						if (prop.second.bVisible == false)
							continue;
						auto type = prop.second.GetTypeName();
						if (type == core::reflection::GetTypeName<glm::vec3>())
						{
							glm::vec3* parameter = prop.second.Get<glm::vec3>(component.get());
							float v[3] = { parameter->x, parameter->y, parameter->z };
							ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
							if (ImGui::InputFloat3(("##" + prop.first + "x").c_str(), v))
							{
								parameter->x = v[0];
								parameter->y = v[1];
								parameter->z = v[2];
								component->OnPropertyChanged(prop.second);
							}
						}
						else if (type == core::reflection::GetTypeName<float>())
						{
							float* parameter = prop.second.Get<float>(component.get());
							if (ImGui::InputFloat(prop.first.c_str(), parameter))
								component->OnPropertyChanged(prop.second);
						}
						else if (type == core::reflection::GetTypeName<int>())
						{
							int* parameter = prop.second.Get<int>(component.get());
							ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
							if (prop.second.isConst)
								ImGui::InputInt(("##Input_" + prop.first).c_str(), parameter, 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
							else
								if (ImGui::InputInt(("##Input_" + prop.first).c_str(), parameter))
									component->OnPropertyChanged(prop.second);
						}
						else if (type == core::reflection::GetTypeName<uint32_t>())
						{
							uint32_t* parameter = prop.second.Get<uint32_t>(component.get());
							ImGui::LabelText(("##" + prop.first).c_str(), prop.first.c_str());
							if (prop.second.isConst)
								ImGui::InputInt(("##Input_" + prop.first).c_str(), reinterpret_cast<int*>(parameter), 0, 0, ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly);
							else
								if(ImGui::InputInt(("##Input_" + prop.first).c_str(), reinterpret_cast<int*>(parameter)))
									component->OnPropertyChanged(prop.second);
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
					obj->AddComponent(components.at(items[current])->Create());
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
		if (bChangedViewportSize)
		{
			bChangedViewportSize = false;
			if (auto tex = world.textures.GetResource("RenderTexture"); tex != nullptr)
			{
				ImGui_ImplVulkan_RemoveTexture(viewportDescSet);
				viewportDescSet = nullptr;
				if(viewportWidthLast != 0.f && viewportHeightLast != 0.f)
					static_cast<render::RenderTexture*>(tex)->SetSize(viewportWidthLast, viewportHeightLast);
			}
		}
		if (viewportDescSet == nullptr)
		{
			if (auto tex = world.textures.GetResource("RenderTexture"); tex != nullptr)
			{
				auto vkTexBuffer = static_cast<render::VulkanTextureBuffer*>(tex->GetBuffer());
				auto imgBuffer = vkTexBuffer->GetImageBuffer();
				viewportDescSet = ImGui_ImplVulkan_AddTexture(imgBuffer->GetSampler(), imgBuffer->GetImageView(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}
	void EditorUI::Render()
	{
		if (!imgui.IsInit())
			return;

		SetDockNode();
		DrawHierarchy();
		DrawInspector();
		DrawProject();
		DrawViewport();

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
	}
}