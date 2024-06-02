#pragma once

#include "Export.h"

#include "glm/vec3.hpp"

#include <vector>
#include <array>
#include <memory>

namespace sh::game
{
	class GameObject;

	class Octree
	{
	private:
		int capacity;
		int minSize;

		glm::vec3 min, max;

		std::array<std::unique_ptr<Octree>, 8> childs;

		std::vector<GameObject*> objs;

		bool hasChild;
	public:
		Octree(glm::vec3 min, glm::vec3 max, int capacity = 100);
		Octree(Octree&& other) noexcept;
		~Octree();

		auto Query(glm::vec3 pos) -> Octree*;
		bool Insert(GameObject* obj);

		auto GetObjs() const -> const std::vector<GameObject*>&;
		auto GetObjs() -> std::vector<GameObject*>&;
	};
}