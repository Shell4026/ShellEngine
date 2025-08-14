#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/ISerializable.h"
#include "Core/UUID.h"
#include "Core/AssetBundle.h"

#include <unordered_map>
#include <filesystem>
namespace sh::core
{
	class Plugin;
}
namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class World;
	class ImGUImpl;

	class GameManager : public core::Singleton<GameManager>, core::ISerializable
	{
		friend core::Singleton<GameManager>;
	public:
		SH_GAME_API void Clean();

		SH_GAME_API void AddWorld(World& world);
		SH_GAME_API void RemoveWorld(const core::UUID& uuid);

		SH_GAME_API auto GetWorld(const core::UUID& uuid) -> World*;
		SH_GAME_API auto GetWorlds() const -> const std::unordered_map<core::UUID, World*>;

		SH_GAME_API void SetCurrentWorld(World& world);
		SH_GAME_API auto GetCurrentWorld() const -> World*;

		SH_GAME_API void SetStartingWorld(World& world);
		SH_GAME_API auto GetStartingWorld() const -> World*;

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		/// @brief 현재 월드를 업데이트 하는 함수. (루프에서 사용)
		SH_GAME_API void UpdateWorld(float dt);

		SH_GAME_API void UnloadWorld(World& world);

		SH_GAME_API auto LoadGame(const std::filesystem::path& managerPath, core::AssetBundle& bundle) -> bool;
	protected:
		SH_GAME_API GameManager() = default;
		SH_GAME_API ~GameManager();
	private:
		void LoadDefaultAsset(core::AssetBundle& bundle);
		void LoadUserModule();
	private:
		World* startingWorld = nullptr;
		World* currentWorld = nullptr;

		std::unordered_map<core::UUID, World*> worlds;
		std::unordered_map<core::UUID, std::vector<core::UUID>> worldUUIDs;

		std::unique_ptr<core::Plugin> userPlugin;
	};
}//namespace