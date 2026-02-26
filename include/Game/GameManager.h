#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/ISerializable.h"
#include "Core/UUID.h"
#include "Core/AssetBundle.h"
#include "Core/Plugin.h"
#include "Core/SContainer.hpp"

#include "Game/World.h"
#include "Game/ImGUImpl.h"
#include "Game/ComponentLoader.h"

#include <unordered_map>
#include <filesystem>
#include <filesystem>
namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class ImGUImpl;

	class GameManager : public core::Singleton<GameManager>
	{
		friend core::Singleton<GameManager>;
	public:
		/// @brief Single = 현재 불러와져 있는 모든 월드를 언로드 하고 불러온다.
		/// @brief Addtive = 현재 불러와져 있는 월드에 추가로 월드를 불러온다.
		enum class LoadMode
		{
			Single,
			Additive
		};
	public:
		SH_GAME_API void Init(render::Renderer& renderer, ImGUImpl& gui);
		SH_GAME_API auto GetRenderer() const -> render::Renderer&;
		SH_GAME_API auto GetUIContext() const -> ImGUImpl&;

		SH_GAME_API void Clean();

		SH_GAME_API auto GetMainWorld() const -> World*;
		SH_GAME_API auto GetWorld(const core::UUID& uuid) const -> World*;
		SH_GAME_API auto GetWorlds() const -> const std::unordered_map<core::UUID, World*> { return worlds; }
		
		/// @brief 현재 로드된 모든 월드를 업데이트 하는 함수. (루프에서 사용)
		SH_GAME_API void UpdateWorlds(double dt);

		/// @brief 월드를 불러온다. 현재 루프가 끝난 후에 작동된다.
		/// @param mode 월드를 불러오는 동작 방식
		/// @param bPlayWorld 월드를 불러오고 난 후 Play()를 작동 시킬 것인지
		/// @param uuid 월드 UUID
		SH_GAME_API void LoadWorld(const core::UUID& uuid, LoadMode mode = LoadMode::Single, bool bPlayWorld = false);
		/// @brief 불러온 월드라면 메모리에서 해제하는 함수
		/// @param uuid 월드 UUID
		SH_GAME_API void UnloadWorld(const core::UUID& uuid);

		SH_GAME_API void LoadUserModule(const std::filesystem::path& path);

		SH_GAME_API void StartWorlds();
		SH_GAME_API void StopWorlds();

		SH_GAME_API auto LoadGame(const std::filesystem::path& managerPath, core::AssetBundle& bundle) -> bool;

		/// @brief 월드가 변경 돼도 유지 되는 오브젝트를 생성한다.
		/// @param name 이름
		/// @return 게임 오브젝트
		SH_GAME_API auto CreateImmortalObject(std::string_view name) -> GameObject&;
		/// @brief 월드가 변경 돼도 유지 되는 오브젝트로 만드는 함수.
		/// @brief 해당 오브젝트 컴포넌트의 생성자에서 호출하면 안 된다!
		/// @brief 이미 불멸 오브젝트면 무시된다.
		/// @param obj 게임 오브젝트
		SH_GAME_API void SetImmortalObject(GameObject& obj);
		SH_GAME_API void ClearImmortalObjects();

		SH_GAME_API void AddAterUpdateTask(const std::function<void()>& fn);
	protected:
		SH_GAME_API GameManager() = default;
		SH_GAME_API ~GameManager();
	private:
		void LoadDefaultAsset(core::AssetBundle& bundle);
	private:
		render::Renderer* renderer = nullptr;
		ImGUImpl* gui = nullptr;

		core::SObjWeakPtr<World> mainWorld = nullptr;

		core::SHashMap<core::UUID, World*> worlds;
		std::unordered_map<core::UUID, std::vector<core::UUID>> worldUUIDs;

		ComponentLoader componentLoader;

		World* loadingSingleWorld = nullptr;
		std::queue<World*> loadingWorldQueue; // Mode = additive인 월드
		std::queue<std::function<void()>> afterUpdateTaskQueue;

		World* immortalWorld = nullptr;

		core::SVector<core::SObject*> defaultAssets;

		bool bLoadingWorld = false;
	};
}//namespace