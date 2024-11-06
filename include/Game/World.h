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
#include "Render/Mesh.h"
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

	class World : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(World)
	private:
		static constexpr float FIXED_TIME = 0.02f;

		core::SVector<GameObject*> objs;
		core::SSet<Camera*> cameras;
		core::SHashMap<std::string, uint32_t> objsMap;
		std::queue<int> objsEmptyIdx;

		std::queue<GameObject*> addedObjQueue; // 루프 도중 추가 된 객체
		core::SHashMap<std::string, GameObject*> addedObjMap;

		float _deltaTime = 0.f;
		float _fixedDeltaTime = 0.f;

		Camera* mainCamera;
		core::GarbageCollection* gc;

		phys::PhysWorld physWorld;

		Octree lightOctree;

		bool startLoop = false;
	public:
		sh::render::Renderer& renderer;
		const float& deltaTime = _deltaTime;
		const float& fixedDeltaTime = _fixedDeltaTime;

		sh::game::ResourceManager<sh::render::Shader> shaders;
		sh::game::ResourceManager<sh::render::Material> materials;
		sh::game::ResourceManager<sh::render::Mesh> meshes;
		sh::game::ResourceManager<sh::render::Texture> textures;
	public:
		const ComponentModule& componentModule;
		const core::SVector<GameObject*>& gameObjects = objs;
		core::Observer<false, Camera*> onCameraAdd;
		core::Observer<false, Camera*> onCameraRemove;
		core::Observer<false, GameObject*> onGameObjectAdded;
		core::Observer<false, GameObject*> onGameObjectRemoved;
	public:
		SH_GAME_API World(sh::render::Renderer& renderer, const ComponentModule& componentModule);
		SH_GAME_API World(World&& other) noexcept;
		SH_GAME_API virtual ~World();

		SH_GAME_API virtual void Clean();

		/// @brief 게임 오브젝트를 추가한다. 이름이 같은 오브젝트가 존재 시 숫자가 붙는다.
		/// @param name 오브젝트 이름
		SH_GAME_API auto AddGameObject(const std::string& name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(const std::string& name);
		SH_GAME_API void DestroyGameObject(const GameObject& obj);
		SH_GAME_API auto ChangeGameObjectName(GameObject& obj, const std::string& to) -> std::string;
		SH_GAME_API auto ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string;
		SH_GAME_API auto GetGameObject(std::string_view name) const -> GameObject*;
		/// @brief obj를 pivotObj보다 배열에서 우선 배치하는 함수
		/// @param obj 바꿀 오브젝트 이름
		/// @param pivotObj 기준 오브젝트 이름
		SH_GAME_API void ReorderObjectAbovePivot(std::string_view obj, std::string_view pivotObj);

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
	};
}