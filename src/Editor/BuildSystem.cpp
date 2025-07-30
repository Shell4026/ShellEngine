#include "BuildSystem.h"
#include "AssetDatabase.h"
#include "Meta.h"
#include "UI/Project.h"

#include "Core/AssetBundle.h"
#include "Core/FileSystem.h"

#include "Game/World.h"
#include "Game/WorldAsset.h"

namespace sh::editor
{
    void BuildSystem::ExtractUUIDs(const core::Json& data)
    {
        if (data.is_object())
        {
            for (auto const& [key, val] : data.items())
            {
                ExtractUUIDs(val);
            }
        }
        else if (data.is_array())
        {
            for (const auto& item : data)
            {
                ExtractUUIDs(item);
            }
        }
        else if (data.is_string())
        {
            const std::string& value = data.get<std::string>();
            if (std::regex_match(value, uuidRegex))
            {
                if (uuids.find(value) == uuids.end())
                {
                    uuids.insert(value);
                    core::SObject* obj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ value });
                    if (core::IsValid(obj))
                        ExtractUUIDs(obj->Serialize());
                }
            }
        }
    }

    void BuildSystem::PackingAssets(game::World& world, const std::filesystem::path& outputPath)
    {
        core::AssetBundle bundle;
        for (const auto& uuid : uuids)
        {
            auto asset = AssetDatabase::GetInstance()->GetAsset(core::UUID{ uuid });
            if (asset != nullptr)
                bundle.AddAsset(*asset, true);
        }

        game::WorldAsset worldAsset{ world };
        bundle.AddAsset(worldAsset, true);

        bundle.SaveBundle(outputPath);
    }

    void BuildSystem::ExportGameManager(const std::filesystem::path& outputPath)
    {
    }

    BuildSystem::BuildSystem() :
        uuidRegex("^[0-9a-f]{32}$", std::regex::optimize)
    {
    }

    void BuildSystem::Build(Project& project, game::World& world, const std::filesystem::path& outputPath)
    {
        currentProject = &project;

        uuids.clear();
        core::Json worldJson = world.Serialize();
        ExtractUUIDs(worldJson);
        PackingAssets(world, outputPath / "assets.bundle");
        ExportGameManager(outputPath / "gameManager.bin");
    }
}//namespace