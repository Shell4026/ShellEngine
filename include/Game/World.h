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
	private:
		static constexpr float FIXED_TIME = 0.02f;

		core::GarbageCollection* gc;
		ImGUImpl* imgui = nullptr;

		core::SHashSet<GameObject*> objs;
		core::SSet<Camera*> cameras;

		std::vector<GameObject*> addedObjs; // 루프 도중 추가 된 객체

		float _deltaTime = 0.f;
		float _fixedDeltaTime = 0.f;

		Camera* mainCamera;

		phys::PhysWorld physWorld;

		Octree lightOctree;

		std::queue<std::function<void()>> afterSyncTasks;

		bool startLoop = false;
	public:
		render::Renderer& renderer;
		const float& deltaTime = _deltaTime;
		const float& fixedDeltaTime = _fixedDeltaTime;

		ResourceManager<sh::render::Shader> shaders;
		ResourceManager<sh::render::Material> materials;
		ResourceManager<render::Model> models;
		ResourceManager<sh::render::Mesh> meshes;
		ResourceManager<sh::render::Texture> textures;
	public:
		const ComponentModule& componentModule;
		const core::SHashSet<GameObject*>& gameObjects = objs;
		core::Observer<false, Camera*> onCameraAdd;
		core::Observer<false, Camera*> onCameraRemove;
		core::Observer<false, GameObject*> onGameObjectAdded;
		core::Observer<false, GameObject*> onGameObjectRemoved;
	protected:
		SH_GAME_API void CleanObjs();
	public:
		SH_GAME_API World(render::Renderer& renderer, const ComponentModule& componentModule, ImGUImpl& guiContext);
		SH_GAME_API World(World&& other) noexcept;
		SH_GAME_API virtual ~World();

		SH_GAME_API virtual void Clean();

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

		SH_GAME_API void RegisterCamera(Camera* cam);
		SH_GAME_API void UnRegisterCamera(Camera* cam);
		SH_GAME_API auto GetCameras() const -> const core::SSet<Camera*>&;
		SH_GAME_API void SetMainCamera(Camera* cam);
		SH_GAME_API	auto GetMainCamera() const -> Camera*;

		SH_GAME_API auto GetPhysWorld() -> phys::PhysWorld*;
		SH_GAME_API auto GetLightOctree() -> Octree&;
		SH_GAME_API auto GetLightOctree() const -> const Octree&;

		SH_GAME_API virtual void Start();
		SH_GAME_API virtual void Update(float deltaTime);
		SH_GAME_API virtual void AfterSync();

		/// @brief 동기화 후에 실행될 작업을 지정한다. 작업은 1회만 실행된다.
		/// @param func 함수
		SH_GAME_API void AddAfterSyncTask(const std::function<void()>& func);

		SH_GAME_API auto Serialize() const->core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API auto GetUiContext() const -> ImGUImpl&;
	};
}