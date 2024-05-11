#pragma once

#include "Export.h"

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace sh::core
{
	class GC;
}

namespace sh::render
{
	class Renderer;
}

namespace sh::game
{
	class GameObject;

	class World
	{
	private:
		std::vector<std::unique_ptr<GameObject>> objs;
		std::unordered_map<std::string, int> objsMap;
		std::queue<int> objsEmptyIdx;

		sh::core::GC& gc;

		float _deltaTime;
	public:
		const float& deltaTime;
		sh::render::Renderer& renderer;
	public:
		SH_GAME_API World(sh::render::Renderer& renderer, sh::core::GC& gc);
		SH_GAME_API ~World();

		SH_GAME_API void Clean();

		SH_GAME_API auto AddGameObject(const std::string& name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(const std::string& name);
		SH_GAME_API auto ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string;

		SH_GAME_API void Start();
		SH_GAME_API void Update(float deltaTime);
	};
}