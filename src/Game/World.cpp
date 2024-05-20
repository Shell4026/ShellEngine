﻿#include "World.h"

#include "GameObject.h"

#include "Core/GC.h"
#include "Core/Util.h"

namespace sh::game
{
	World::World(sh::render::Renderer& renderer, sh::core::GC& gc) :
		renderer(renderer),gc(gc), deltaTime(_deltaTime),
		_deltaTime(0.0f), 
		shaders(gc, renderer), materials(gc, renderer), meshes(gc, renderer),
		mainCamera(nullptr)
	{
		gc.AddObject(this);
	}
	World::World(World&& other) noexcept :
		renderer(other.renderer), gc(other.gc), deltaTime(_deltaTime),
		_deltaTime(other._deltaTime),
		shaders(std::move(other.shaders)), materials(std::move(other.materials)), meshes(std::move(other.meshes)),
		mainCamera(nullptr)
	{
		gc.RemoveObject(&other);
		gc.AddObject(this);
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
			objsMap.insert(std::make_pair(objName, objs.size() - 1));
			auto obj = objs[objs.size() - 1].get();
			obj->SetGC(gc);

			return obj;
		}
		else
		{
			int idx = objsEmptyIdx.front();
			objsMap.insert(std::make_pair(objName, idx));
			objs[idx] = std::make_unique<GameObject>(*this, objName);
			objsEmptyIdx.pop();

			auto obj = objs[idx].get();
			obj->SetGC(gc);

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
		objs[id].reset();
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
}