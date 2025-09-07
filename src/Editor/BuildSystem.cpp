#include "BuildSystem.h"
#include "AssetDatabase.h"
#include "Meta.h"
#include "UI/Project.h"
#include "EditorResource.h"

#include "Core/AssetBundle.h"
#include "Core/FileSystem.h"

#include "Game/World.h"
#include "Game/GameManager.h"

#include "Game/Asset/WorldAsset.h"
#include "Game/Asset/ShaderAsset.h"
#include "Game/Asset/MaterialAsset.h"
#include "Game/Asset/MeshAsset.h"

#include <fstream>
namespace sh::editor
{
    void BuildSystem::ExtractUUIDs(std::unordered_set<std::string>& set, const core::Json& worldJson)
    {
        if (worldJson.is_object())
        {
            for (auto const& [key, val] : worldJson.items())
            {
                ExtractUUIDs(set, val);
            }
        }
        else if (worldJson.is_array())
        {
            for (const auto& item : worldJson)
            {
                ExtractUUIDs(set, item);
            }
        }
        else if (worldJson.is_string())
        {
            const std::string& value = worldJson.get<std::string>();
            if (std::regex_match(value, uuidRegex))
            {
                if (set.find(value) == set.end())
                {
                    set.insert(value);
                    core::SObject* obj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ value });
                    if (core::IsValid(obj))
                    {
                        ExtractUUIDs(set,obj->Serialize());
                    }
                }
            }
        }
    }

    void BuildSystem::PackingAssets(core::AssetBundle& bundle, game::World& world)
    {
        auto editorResource = EditorResource::GetInstance();
        game::ShaderAsset errorShaderAsset{ *editorResource->GetShader("ErrorShader") };
        bundle.AddAsset(errorShaderAsset, true);
        game::ShaderAsset lineShaderAsset{ *editorResource->GetShader("Line") };
        bundle.AddAsset(lineShaderAsset, true);
        game::MaterialAsset errorMatAsset{ *editorResource->GetMaterial("ErrorMaterial") };
        bundle.AddAsset(errorMatAsset, true);
        game::MaterialAsset lineMatAsset{ *editorResource->GetMaterial("LineMaterial") };
        bundle.AddAsset(lineMatAsset, true);
        game::MeshAsset cubeMesh{ *editorResource->GetModel("CubeModel")->GetMeshes()[0] };
        bundle.AddAsset(cubeMesh, true);
        game::MeshAsset sphereMesh{ *editorResource->GetModel("SphereModel")->GetMeshes()[0] };
        bundle.AddAsset(sphereMesh, true);
        game::MeshAsset planeMesh{ *editorResource->GetModel("PlaneModel")->GetMeshes()[0] };
        bundle.AddAsset(planeMesh, true);

        for (const auto& uuid : uuids)
        {
            if (world.GetUUID() == core::UUID{ uuid })
                continue;
            auto asset = AssetDatabase::GetInstance()->GetAsset(core::UUID{ uuid });
            if (asset != nullptr)
                bundle.AddAsset(*asset, true);
        }

        game::WorldAsset worldAsset{ world };
        worldAsset.ConvertToGameWorldType();
        bundle.AddAsset(worldAsset, true);
    }

    void BuildSystem::ExportGameManager(const std::filesystem::path& outputPath)
    {
        game::GameManager& manager = *game::GameManager::GetInstance();
        ProjectSetting& projectSetting = currentProject->GetProjectSetting();

        manager.SetStartingWorld(*projectSetting.startingWorld);

        core::Json mainJson{};

        mainJson["manager"] = manager.Serialize();
        for (const auto& [worldUUIDStr, uuids] : worldUUIDs)
        {
            core::Json worldUUIDs{};
            for (const auto& uuid : uuids)
            {
                worldUUIDs[worldUUIDStr].push_back(uuid);
            }
            mainJson["uuids"].push_back(std::move(worldUUIDs));
        }

        const std::vector<uint8_t> data{ core::Json::to_bson(mainJson) };

        std::ofstream of{ outputPath, std::ios_base::binary };
        if (!of.is_open())
        {
            SH_ERROR_FORMAT("Failed to export game setting!: {}", outputPath.u8string());
            return;
        }
        of.write(reinterpret_cast<const char*>(data.data()), data.size());
        of.close();
    }

    BuildSystem::BuildSystem() :
        uuidRegex("^[0-9a-f]{32}$", std::regex::optimize)
    {
    }

    void BuildSystem::Build(Project& project, const std::filesystem::path& outputPath)
    {
        currentProject = &project;

        uuids.clear();

        core::AssetBundle bundle;
        auto worldPtr = project.GetProjectSetting().startingWorld.Get();

        core::Json worldJson{};
        if (worldPtr->IsLoaded())
            worldJson = worldPtr->Serialize();
        else
            worldJson = worldPtr->GetWorldPoint();

        std::unordered_set<std::string> uuids;
        ExtractUUIDs(uuids, worldJson);
        for (const auto& uuidStr : uuids)
        {
            this->uuids.insert(uuidStr);
            worldUUIDs[worldPtr->GetUUID().ToString()].push_back(uuidStr);
        }
        PackingAssets(bundle, *worldPtr);

        bundle.SaveBundle(outputPath / "assets.bundle");

        ExportGameManager(outputPath / "gameManager.bin");
    }
}//namespace