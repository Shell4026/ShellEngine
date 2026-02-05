#include "UI/ProjectSettingUI.h"
#include "ProjectSetting.h"
#include "AssetDatabase.h"

namespace sh::editor
{
	SH_EDITOR_API void ProjectSettingUI::RenderUI(ProjectSetting& setting, const std::filesystem::path& rootPath, bool& bOpen)
	{
		static auto& assetDatabase = *AssetDatabase::GetInstance();

		ImGui::SetNextWindowSize(ImVec2{ 512, 512 }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::Begin("Project Setting", &bOpen);

		ImGui::Text("Starting world");
		std::string startingWorldStr = setting.startingWorld == nullptr ? "None" : setting.startingWorld->GetName().ToString();
		ImGui::Button(startingWorldStr.c_str(), ImVec2{ -1, 20 });
		if (ImGui::BeginDragDropTarget())
		{
			const std::string worldType{ core::reflection::TypeTraits::GetTypeName<game::World>() };

			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(worldType.c_str());
			if (payload != nullptr)
			{
				core::SObject* sobjPtr = *reinterpret_cast<core::SObject**>(payload->Data);
				auto pathOpt = assetDatabase.GetAssetOriginalPath(sobjPtr->GetUUID());
				if (pathOpt.has_value())
				{
					setting.startingWorld = static_cast<game::World*>(sobjPtr);
					setting.Save(rootPath / "ProjectSetting.json");
				}
			}
			else
			{
				const ImGuiPayload* currentPayload = ImGui::GetDragDropPayload();
				core::SObject* sobjPtr = *reinterpret_cast<core::SObject**>(currentPayload->Data);
				if (sobjPtr->GetType().IsChildOf(game::World::GetStaticType()))
				{
					payload = ImGui::AcceptDragDropPayload(currentPayload->DataType);
					if (payload != nullptr)
					{
						auto pathOpt = assetDatabase.GetAssetOriginalPath(sobjPtr->GetUUID());
						if (pathOpt.has_value())
						{
							setting.startingWorld = static_cast<game::World*>(sobjPtr);
							setting.Save(rootPath / "ProjectSetting.json");
						}
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::End();
	}
}//namespace