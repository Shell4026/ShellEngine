#include "World.h"

#include "GameObject.h"

namespace sh::game
{
	World::World(sh::render::Renderer& renderer) :
		renderer(renderer),
		_deltaTime(0.0f), deltaTime(_deltaTime)
	{

	}
	World::~World()
	{

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
			if (!obj->activeSelf)
				continue;
			obj->Update();
		}
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->LateUpdate();
		}
	}
}