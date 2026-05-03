#pragma once
#include "Export.h"
#include "ComponentModule.h"
#include "Octree.h"
#include "GameObject.h"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/Memory/MemoryPool.hpp"
#include "Core/EventBus.h"
#include "Core/EventSubscriber.h"
#include "Core/Factory.hpp"

#include "Render/Shader.h"
#include "Render/Material.h"
#include "Render/Model.h"
#include "Render/Texture.h"

#include "Physics/PhysWorld.h"
#include "Physics/PhysicsEvent.h"

#include <string>
#include <vector>
#include <set>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace sh::core
{
	class GarbageCollection;
}
namespace sh::render
{
	class Renderer;
	class ScriptableRenderer;
	class ShadowMapManager;
}

namespace sh::game
{
	class Component;
	class ImGUImpl;
	class Camera;

	class World : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(World)
	public:
		static constexpr float FIXED_TIME = 0.0166f; // 60hz
		using Factory = core::Factory<game::World, game::World*>;
	public:
		SH_GAME_API World(render::Renderer& renderer, ImGUImpl& guiContext);
		SH_GAME_API World(World&& other) = delete;
		SH_GAME_API virtual ~World();

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API virtual void Clear();

		SH_GAME_API virtual void SetupRenderer();
		/// @brief 기본 리소스를 로드한다.
		SH_GAME_API virtual void InitResource();

		/// @brief 게임 오브젝트를 추가한다.
		/// @param name 오브젝트 이름
		SH_GAME_API virtual auto AddGameObject(std::string_view name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(std::string_view name);
		SH_GAME_API void DestroyGameObject(GameObject& obj);
		/// @brief 가장 먼저 발견 된 해당 이름을 가진 게임 오브젝트를 반환하는 함수 O(N)
		/// @param name 이름
		/// @return 못 찾을 시 nullptr, 찾을 시 게임 오브젝트 포인터
		SH_GAME_API auto GetGameObject(std::string_view name) const -> GameObject*;
		/// @brief 게임 오브젝트가 할당된 메모리를 반환 큐에 넣는 함수.
		/// @param ptr 오브젝트 포인터
		SH_GAME_API void PushDeAllocatedGameObject(GameObject* ptr);

		SH_GAME_API void RegisterCamera(Camera& cam);
		SH_GAME_API void UnRegisterCamera(Camera& cam);
		SH_GAME_API void SetMainCamera(Camera* cam);

		SH_GAME_API virtual void Start();
		SH_GAME_API virtual void Update(double deltaTime);
		SH_GAME_API virtual void BeforeSync();
		SH_GAME_API virtual void AfterSync();
		/// @brief 동기화 전에 실행될 작업을 지정한다. 작업은 1회만 실행된다.
		/// @param func 함수
		SH_GAME_API void AddBeforeSyncTask(const std::function<void()>& func);
		/// @brief 동기화 후에 실행될 작업을 지정한다. 작업은 1회만 실행된다.
		/// @param func 함수
		SH_GAME_API void AddAfterSyncTask(const std::function<void()>& func);

		SH_GAME_API void SaveWorldPoint(const core::Json& json, std::string_view name = "default");
		SH_GAME_API void SaveWorldPoint(core::Json&& json, std::string_view name = "default");
		SH_GAME_API void LoadWorldPoint();
		SH_GAME_API void LoadWorldPoint(const std::string& name);
		SH_GAME_API auto GetWorldPoint() const -> const core::Json*;
		SH_GAME_API auto GetWorldPoint(const std::string& name) const -> const core::Json*;
		SH_GAME_API auto GetWorldPoint() -> core::Json*;
		SH_GAME_API auto GetWorldPoint(const std::string& name) -> core::Json*;
		SH_GAME_API void ClearWorldPoints();
		SH_GAME_API void ClearWorldPoint(const std::string& name = "default");

		SH_GAME_API void PublishEvent(const core::IEvent& event);
		SH_GAME_API void SubscribeEvent(core::ISubscriber& subscriber);

		SH_GAME_API void Play();
		SH_GAME_API void Stop();

		SH_GAME_API virtual void ReallocateUUIDS();

		auto GetUiContext() const -> ImGUImpl& { return *imgui; }
		auto GetPhysWorld() -> phys::PhysWorld& { return physWorld; }
		auto GetLightOctree() -> Octree& { return lightOctree; }
		auto GetLightOctree() const -> const Octree& { return lightOctree; }
		auto GetMainCamera() const -> Camera* { return mainCamera; }
		auto GetShadowMapManager() -> render::ShadowMapManager& { return *shadowMapManager; }
		auto GetShadowMapManager() const -> const render::ShadowMapManager& { return *shadowMapManager; }
		auto GetGameObjects() const -> const std::vector<GameObject*>& { return objs; }
		auto GetGameObjectPool() -> core::memory::MemoryPool<GameObject>& { return objPool; }
		auto GetCameras() const -> const std::vector<Camera*>& { return cameras; }
		auto IsPlaying() const -> bool { return bPlaying; }
		auto IsStart() const -> bool { return bOnStart; }
		auto IsLoaded() const -> bool { return bLoaded; }
		auto GetFixedAccumulator() const -> double { return dtAccumulator; }
	protected:
		SH_GAME_API void CleanObjs();
	private:
		auto AllocateGameObject() -> GameObject*;
	public:
		render::Renderer& renderer;
		const double& deltaTime = dt;
	public:
		const ComponentModule& componentModule;
	protected:
		core::EventBus eventBus;
		std::unordered_map<GameObject*, std::size_t> objIdx;
		std::vector<GameObject*> objs;
		std::vector<Camera*> cameras;

		std::unique_ptr<render::ScriptableRenderer> customRenderer;
		std::unique_ptr<render::ShadowMapManager> shadowMapManager;
	private:
		core::GarbageCollection* gc;
		ImGUImpl* imgui = nullptr;

		core::memory::MemoryPool<GameObject> objPool;

		std::vector<core::SObjWeakPtr<GameObject>> addedObjs; // 루프 도중 추가 된 객체

		double dt = 0.0;
		double dtAccumulator = 0.0;

		PROPERTY(mainCamera, core::PropertyOption::sobjPtr)
		Camera* mainCamera = nullptr;

		phys::PhysWorld physWorld;

		Octree lightOctree;

		std::queue<std::function<void()>> beforeSyncTasks;
		std::queue<std::function<void()>> afterSyncTasks;
		std::queue<GameObject*> deallocatedObjs;

		std::unordered_map<std::string, core::Json> savePoints;

		core::EventSubscriber<phys::PhysicsEvent> physEventSubscriber;

		bool bStartLoop = false;
		bool bWaitPlaying = false;
		bool bPlaying = false;
		bool bLoaded = false;
		bool bOnStart = false;
	};
}//namespace