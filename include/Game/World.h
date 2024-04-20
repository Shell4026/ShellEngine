#pragma once

#include "Export.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string_view>
namespace sh::game
{
	class GameObject;

	class World
	{
	private:
		std::vector<std::unique_ptr<GameObject>> objs;
		std::unordered_map<std::string, int> objsMap;
		std::queue<int> objsEmptyIdx;
	public:
		SH_GAME_API World();
		SH_GAME_API ~World();

		SH_GAME_API auto AddGameObject(const std::string& name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(const std::string& name);
		SH_GAME_API auto ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string;

		SH_GAME_API void Start();
		SH_GAME_API void Update();
	};
}