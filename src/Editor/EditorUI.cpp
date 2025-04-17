#include "EditorUI.h"
#include "Project.h"
#include "EditorWorld.h"

#include "game/GameObject.h"
#include "game/World.h"

#include "Render/VulkanImpl/VulkanTextureBuffer.h"
#include "Render/RenderTexture.h"

#include <cstring>

namespace sh::editor
{
	using namespace sh::game;

	EditorUI::EditorUI(EditorWorld& world, game::ImGUImpl& imgui) :
		UI(imgui),
		world(world),
		viewport(imgui, world),
		hierarchy(imgui, world),
		project(imgui, world),
		inspector(imgui, world),

		hierarchyWidth(0), hierarchyHeight(0),
		explorer(imgui),

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
			ImGui::DockBuilderDockWindow(Inspector::name, dockRight);
			ImGui::DockBuilderDockWindow(Project::name, dockDown);
			ImGui::DockBuilderDockWindow(this->viewport.name, dockspaceId);
			ImGui::DockBuilderFinish(dockspaceId);
		}
		ImGui::End();
	}

	inline void EditorUI::DrawMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New project"))
				{
					explorer.AddCallback([&](std::filesystem::path dir)
						{
							project.CreateNewProject(dir);
						}
					);
					explorer.Open(ExplorerUI::OpenMode::Create);
				}
				if (ImGui::MenuItem("Open project"))
				{
					explorer.AddCallback([&](std::filesystem::path dir)
						{
							project.OpenProject(dir);
						}
					);
					explorer.Open();
				}
				if (ImGui::MenuItem("Save world", "Ctrl+S"))
				{
					project.SaveWorld();
				}
				if (ImGui::MenuItem("Load world", "Ctrl+O"))
				{
					project.LoadWorld();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	SH_EDITOR_API void EditorUI::Update()
	{
		viewport.Update();
		hierarchy.Update();
		inspector.Update();
		project.Update();
	}

	SH_EDITOR_API void EditorUI::Render()
	{
		if (!imgui.IsInit())
			return;

		SetDockNode();
		hierarchy.Render();
		inspector.Render();
		project.Render();
		viewport.Render();
	
		DrawMenu();
		explorer.Render();

		imgui.SyncDirty();
	}

	SH_EDITOR_API auto EditorUI::GetViewport() -> Viewport&
	{
		return viewport;
	}

	SH_EDITOR_API auto EditorUI::GetHierarchy() -> Hierarchy&
	{
		return hierarchy;
	}

	SH_EDITOR_API void EditorUI::Clean()
	{
		viewport.Clean();
	}
}