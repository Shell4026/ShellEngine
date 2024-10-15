#include "PCH.h"
#include "World.h"

#include "GameObject.h"
#include "Component/Camera.h"

#include "Core/GarbageCollection.h"
#include "Core/Util.h"

#include <utility>
#include <cstdint>

namespace sh::game
{
	SH_GAME_API World::World(sh::render::Renderer& renderer, const ComponentModule& componentModule) :
		renderer(renderer), componentModule(componentModule),
		deltaTime(_deltaTime), gameObjects(objs),
		
		_deltaTime(0.0f), 
		shaders(renderer), materials(renderer), meshes(renderer), textures(renderer),
		mainCamera(nullptr)
	{
		gc = core::GarbageCollection::GetInstance();

		gc->SetRootSet(this);
	}
	SH_GAME_API World::World(World&& other) noexcept :
		renderer(other.renderer), gc(other.gc), componentModule(other.componentModule),
		deltaTime(_deltaTime), gameObjects(objs),
		
		_deltaTime(other._deltaTime), 
		objs(std::move(other.objs)), objsMap(std::move(objsMap)), objsEmptyIdx(std::move(other.objsEmptyIdx)),
		shaders(std::move(other.shaders)), materials(std::move(other.materials)), meshes(std::move(other.meshes)), textures(std::move(other.textures)),
		mainCamera(nullptr)
	{
		gc->RemoveRootSet(&other);
		gc->SetRootSet(this);
	}
	SH_GAME_API World::~World()
	{
		Clean();
	}

	SH_GAME_API void World::Clean()
	{
		meshes.Clean();
		materials.Clean();
		shaders.Clean();
		textures.Clean();

		objsEmptyIdx = std::queue<int>{};
		objsMap.clear();
		objs.clear();
	}

	SH_GAME_API auto World::AddGameObject(const std::string& name) -> GameObject*
	{
		std::string objName = name;

		//중복되는 이름이 있으면 뒤에 숫자를 붙인다.
		int nameIdx = 0;
		auto it = objsMap.find(objName);
		while (it != objsMap.end())
		{
			objName = name + std::to_string(nameIdx++);
			it = objsMap.find(objName);
		}

		if (objsEmptyIdx.empty())
		{
			objs.push_back(Create<GameObject>(*this, objName));
			objsMap.insert(std::make_pair(objName, static_cast<uint32_t>(objs.size() - 1)));

			auto obj = objs[objs.size() - 1];
			onGameObjectAdded.Notify(obj);

			return obj;
		}
		else
		{
			int idx = objsEmptyIdx.front();
			objsMap.insert(std::make_pair(objName, idx));
			objs[idx] = Create<GameObject>(*this, objName);
			objsEmptyIdx.pop();

			auto obj = objs[idx];
			onGameObjectAdded.Notify(obj);

			return obj;
		}
	}

	SH_GAME_API void World::DestroyGameObject(const std::string& name)
	{
		auto it = objsMap.find(name);
		if (it == objsMap.end())
			return;

		int id = it->second;
		objsMap.erase(it);
		objsEmptyIdx.push(id);
		objs[id]->Destroy();

		onGameObjectRemoved.Notify(objs[id]);
	}
	SH_GAME_API void World::DestroyGameObject(const GameObject& obj)
	{
		DestroyGameObject(obj.name);
	}

	SH_GAME_API auto World::ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string
	{
		auto it = objsMap.find(objName);
		if (it == objsMap.end())
			return "";

		std::string name{ to };
		int nameIdx = 0;
		it = objsMap.find(to);
		while (it == objsMap.end())
		{
			name = to + std::to_string(nameIdx++);
			it = objsMap.find(objName);
		}

		int id = it->second;
		objsMap.insert(std::make_pair(name, id));
		objsMap.erase(it);

		return name;
	}
	SH_GAME_API auto World::ChangeGameObjectName(GameObject& obj, const std::string& to) -> std::string
	{
		return ChangeGameObjectName(obj.name, to);
	}

	SH_GAME_API auto World::GetGameObject(std::string_view name) const -> GameObject*
	{
		auto it = objsMap.find(std::string{ name });
		if (it == objsMap.end())
			return nullptr;
		return objs[it->second];
	}

	SH_GAME_API void World::ReorderObjectAbovePivot(std::string_view obj, std::string_view pivotObj)
	{
		if (obj == pivotObj)
			return;
		auto objIt = objsMap.find(std::string{ obj });
		if (objIt == objsMap.end())
			return;
		auto pivotIt = objsMap.find(std::string{ pivotObj });
		if (pivotIt == objsMap.end())
			return;

		uint32_t objIdx = objIt->second;
		uint32_t pivotIdx = pivotIt->second;
		
		if (objIdx > pivotIdx)
		{
			pivotIt->second++;
			objIt->second = pivotIdx;
			auto objPtr = objs[objIdx];
			for (std::size_t i = objIdx; i > pivotIdx; --i)
			{
				objsMap[objs[i - 1]->name] = i;
				objs[i] = objs[i - 1];
			}
			objs[pivotIdx] = objPtr;
		}
		else
		{
			auto objPtr = objs[objIdx];
			objIt->second = pivotIdx - 1;
			for (std::size_t i = objIdx; i < pivotIdx - 1; ++i)
			{
				objsMap[objs[i + 1]->name] = i;
				objs[i] = objs[i + 1];
			}
			objs[pivotIdx - 1] = objPtr;
		}
	}

	SH_GAME_API void World::Start()
	{
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->Awake();
		}
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->OnEnable();
		}
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->Start();
		}
	}
	SH_GAME_API void World::Update(float deltaTime)
	{
		_deltaTime = deltaTime;

		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->BeginUpdate();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->Update();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->LateUpdate();
		}
	}

	SH_GAME_API void World::RegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.insert(cam);

		onCameraAdd.Notify(cam);
	}
	SH_GAME_API void World::UnRegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.erase(cam);

		onCameraRemove.Notify(cam);
	}
	SH_GAME_API auto World::GetCameras() const -> const core::SSet<Camera*>&
	{
		return cameras;
	}
	SH_GAME_API void World::SetMainCamera(Camera* cam)
	{
		if (core::IsValid(cam))
		{
			mainCamera = cam;
		}
	}
	SH_GAME_API auto World::GetMainCamera() const -> Camera*
	{
		return mainCamera;
	}
}