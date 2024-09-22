﻿#include "World.h"

#include "GameObject.h"
#include "Component/Camera.h"

#include "Core/GarbageCollection.h"
#include "Core/Util.h"

#include <utility>
#include <cstdint>

namespace sh::game
{
	World::World(sh::render::Renderer& renderer, const ComponentModule& componentModule) :
		renderer(renderer), componentModule(componentModule),
		deltaTime(_deltaTime), gameObjects(objs),
		
		_deltaTime(0.0f), 
		shaders(renderer), materials(renderer), meshes(renderer), textures(renderer),
		mainCamera(nullptr)
	{
		gc = core::GarbageCollection::GetInstance();

		gc->SetRootSet(this);
	}
	World::World(World&& other) noexcept :
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
	World::~World()
	{
		Clean();
	}

	void World::Clean()
	{
		meshes.Clean();
		materials.Clean();
		shaders.Clean();
		textures.Clean();

		objsEmptyIdx = std::queue<int>{};
		objsMap.clear();
		objs.clear();
	}

	auto World::AddGameObject(const std::string& name) -> GameObject*
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
			objs.push_back(std::make_unique<GameObject>(*this, objName));
			objsMap.insert(std::make_pair(objName, static_cast<uint32_t>(objs.size() - 1)));
			auto obj = objs[objs.size() - 1].get();
			gc->SetRootSet(obj);

			return obj;
		}
		else
		{
			int idx = objsEmptyIdx.front();
			objsMap.insert(std::make_pair(objName, idx));
			objs[idx] = std::make_unique<GameObject>(*this, objName);
			objsEmptyIdx.pop();

			auto obj = objs[idx].get();
			gc->SetRootSet(obj);

			return obj;
		}
	}

	void World::DestroyGameObject(const std::string& name)
	{
		auto it = objsMap.find(name);
		if (it == objsMap.end())
			return;

		int id = it->second;
		objsMap.erase(it);
		objsEmptyIdx.push(id);
		objs[id]->Destroy();
		objs[id].release();
	}

	auto World::ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string
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
	auto World::GetGameObject(std::string_view name) const -> GameObject*
	{
		auto it = objsMap.find(std::string{ name });
		if (it == objsMap.end())
			return nullptr;
		return objs[it->second].get();
	}

	void World::Start()
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
	void World::Update(float deltaTime)
	{
		_deltaTime = deltaTime;

		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj.get()))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->Update();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj.get()))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->LateUpdate();
		}
	}

	void World::RegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.insert(cam);

		onCameraAdd.Notify(cam);
	}
	void World::UnRegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.erase(cam);

		onCameraRemove.Notify(cam);
	}
	auto World::GetCameras() const -> const core::SSet<Camera*>&
	{
		return cameras;
	}
	void World::SetMainCamera(Camera* cam)
	{
		if (core::IsValid(cam))
		{
			mainCamera = cam;
		}
	}
	auto World::GetMainCamera() const -> Camera*
	{
		return mainCamera;
	}
}