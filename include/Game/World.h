#pragma once
#include "Export.h"
#include "ResourceManager.hpp"
#include "ComponentModule.h"
#include "Octree.h"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"
#include "Core/Memory/MemoryPool.hpp"
#include "Core/EventBus.h"
#include "Core/AssetBundle.h"

#include "Render/Shader.h"
#include "Render/Material.h"
#include "Render/Model.h"
#include "Render/Texture.h"

#include "Physics/PhysWorld.h"

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
}

namespace sh::game
{
	class GameObject;
	class Camera;
	class ImGUImpl;

	class World : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(World)
	public:
		static constexpr float FIXED_TIME = 0.0166f; // 60hz
	public:
		SH_GAME_API World(render::Renderer& renderer, ImGUImpl& guiContext);
		SH_GAME_API World(World&& other) = delete;
		SH_GAME_API virtual ~World();

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API virtual void Clean();

		/// @brief 기본 리소스를 로드한다.
		SH_GAME_API virtual void InitResource();
		SH_GAME_API virtual void SetRenderPass();

		SH_GAME_API auto LoadAssetBundle(const std::filesystem::path& path) -> bool;
		SH_GAME_API auto LoadObjectFromBundle(const core::UUID& uuid) -> core::SObject*;

		/// @brief 게임 오브젝트를 추가한다.
		/// @param name 오브젝트 이름
		SH_GAME_API virtual auto AddGameObject(std::string_view name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(std::string_view name);
		SH_GAME_API void DestroyGameObject(GameObject& obj);
		/// @brief 가장 먼저 발견 된 해당 이름을 가진 게임 오브젝트를 반환하는 함수 O(N)
		/// @param name 이름
		/// @return 못 찾을 시 nullptr, 찾을 시 게임 오브젝트 포인터
		SH_GAME_API auto GetGameObject(std::string_view name) const -> GameObject*;
		SH_GAME_API auto GetGameObjects() const -> const core::SHashSet<GameObject*>&;
		SH_GAME_API auto GetGameObjectPool() -> core::memory::MemoryPool<GameObject>&;
		/// @brief 게임 오브젝트가 할당된 메모리를 반환 큐에 넣는 함수.
		/// @param ptr 오브젝트 포인터
		SH_GAME_API void PushDeAllocatedGameObject(GameObject* ptr);
		/// @brief 게임 오브젝트를 복제한다.
		/// @param obj 게임 오브젝트
		/// @return 복제된 게임 오브젝트
		SH_GAME_API virtual auto DuplicateGameObject(const GameObject& obj) -> GameObject&;

		SH_GAME_API void RegisterCamera(Camera* cam);
		SH_GAME_API void UnRegisterCamera(Camera* cam);
		SH_GAME_API auto GetCameras() const -> const std::unordered_set<Camera*>&;
		SH_GAME_API void SetMainCamera(Camera* cam);
		SH_GAME_API	auto GetMainCamera() const -> Camera*;

		SH_GAME_API auto GetPhysWorld() -> phys::PhysWorld*;
		SH_GAME_API auto GetLightOctree() -> Octree&;
		SH_GAME_API auto GetLightOctree() const -> const Octree&;

		SH_GAME_API virtual void Start();
		SH_GAME_API void Update(float deltaTime);
		SH_GAME_API virtual void BeforeSync();
		SH_GAME_API virtual void AfterSync();
		/// @brief 동기화 전에 실행될 작업을 지정한다. 작업은 1회만 실행된다.
		/// @param func 함수
		SH_GAME_API void AddBeforeSyncTask(const std::function<void()>& func);
		/// @brief 동기화 후에 실행될 작업을 지정한다. 작업은 1회만 실행된다.
		/// @param func 함수
		SH_GAME_API void AddAfterSyncTask(const std::function<void()>& func);

		SH_GAME_API auto Serialize() const->core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API void SaveWorldPoint(const core::Json& json);
		SH_GAME_API void SaveWorldPoint(core::Json&& json);
		SH_GAME_API void LoadWorldPoint();
		SH_GAME_API auto GetWorldPoint() const -> const core::Json&;

		SH_GAME_API auto GetUiContext() const -> ImGUImpl&;

		SH_GAME_API void PublishEvent(const core::IEvent& event);
		SH_GAME_API void SubscribeEvent(core::ISubscriber& subscriber);

		SH_GAME_API void Play();
		SH_GAME_API void Stop();

		SH_GAME_API auto IsPlaying() const -> bool;
		SH_GAME_API auto IsStart() const -> bool;
		SH_GAME_API auto IsLoaded() const -> bool;

		SH_GAME_API virtual void ReallocateUUIDS();
	protected:
		SH_GAME_API void CleanObjs();
	private:
		auto AllocateGameObject() -> GameObject*;
	public:
		render::Renderer& renderer;
		const float& deltaTime = _deltaTime;
		const float& fixedDeltaTime = _fixedDeltaTime;

		ResourceManager<render::Shader> shaders;
		ResourceManager<render::Material> materials;
		ResourceManager<render::Model> models;
		ResourceManager<render::Texture> textures;
	public:
		const ComponentModule& componentModule;
	protected:
		core::EventBus eventBus;
	private:
		core::GarbageCollection* gc;
		ImGUImpl* imgui = nullptr;

		core::memory::MemoryPool<GameObject> objPool;

		core::SHashSet<GameObject*> objs;
		core::SHashSet<Camera*> cameras;

		std::vector<GameObject*> addedObjs; // 루프 도중 추가 된 객체

		float _deltaTime = 0.f;
		float _fixedDeltaTime = 0.f;

		PROPERTY(mainCamera)
		Camera* mainCamera;

		phys::PhysWorld physWorld;

		Octree lightOctree;

		std::queue<std::function<void()>> beforeSyncTasks;
		std::queue<std::function<void()>> afterSyncTasks;
		std::queue<GameObject*> deallocatedObjs;

		core::AssetBundle assetBundle;

		core::Json lateSerializedData;

		bool bStartLoop = false;
		bool bPlaying = false;
		bool bLoaded = false;
		bool bOnStart = false;
	};
}