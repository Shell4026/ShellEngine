#include "ProjectSetting.h"
#include "AssetDatabase.h"

#include "Core/Util.h"
#include "Core/Logger.h"
#include "Core/FileSystem.h"

#include "Game/GameManager.h"

#include <fstream>
#include <optional>
namespace sh::editor
{
	ProjectSetting::ProjectSetting() :
		version(0)
	{
	}
	ProjectSetting::~ProjectSetting()
	{
	}
	SH_EDITOR_API auto ProjectSetting::Serialize() const -> core::Json
	{
		core::Json json;
		json["version"] = version;
		if (startingWorld.IsValid())
			json["startingWorld"] = startingWorld->GetUUID().ToString();
		return json;
	}
	SH_EDITOR_API void ProjectSetting::Deserialize(const core::Json& json)
	{
		if (json.contains("version"))
			version = json["version"];
		if (json.contains("startingWorld"))
		{
			core::SObject* worldObj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ json["startingWorld"].get<std::string>()});
			if (core::IsValid(worldObj))
				startingWorld = static_cast<game::World*>(worldObj);
		}
	}
	SH_EDITOR_API void ProjectSetting::Save(const std::filesystem::path& path)
	{
		SH_INFO_FORMAT("Save project setting: {}", path.u8string());
		if (std::ofstream os{ path })
		{
			os << std::setw(4) << Serialize();
			os.close();
		}
		else
			SH_ERROR_FORMAT("Save error! {}", path.u8string());
	}
	SH_EDITOR_API void ProjectSetting::Load(const std::filesystem::path& path)
	{
		auto stringOpt = core::FileSystem::LoadText(path);
		if (stringOpt.has_value())
		{
			if (stringOpt.value().empty())
				Save(path);
			else
			{
				Deserialize(core::Json::parse(stringOpt.value()));
				if (startingWorld.IsValid())
				{
					auto gameManager = game::GameManager::GetInstance();
					gameManager->SetStartingWorld(*startingWorld);
					gameManager->AddWorld(*startingWorld);
				}
			}
		}
		else
			Save(path);
	}
	SH_EDITOR_API void ProjectSetting::RenderUI(bool& bOpen, const std::filesystem::path& rootPath)
	{
		auto& assetDatabase = *AssetDatabase::GetInstance();

		ImGui::SetNextWindowSize(ImVec2{ 512, 512 }, ImGuiCond_::ImGuiCond_Appearing);
		ImGui::Begin("Project Setting", &bOpen);

		ImGui::Text("Starting world");
		std::string startingWorldStr = startingWorld == nullptr ? "None" : startingWorld->GetName().ToString();
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
					startingWorld = static_cast<game::World*>(sobjPtr);
					Save(rootPath / "ProjectSetting.json");
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
							startingWorld = static_cast<game::World*>(sobjPtr);
							Save(rootPath / "ProjectSetting.json");
						}
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::End();
	}
}
