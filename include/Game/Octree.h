#pragma once
#include "Export.h"
#include "Vector.h"

#include "Render/AABB.h"

#include <array>
#include <memory>
#include <vector>

namespace sh::game
{
	class IOctreeElement;

	class Octree
	{
	public:
		SH_GAME_API Octree(const render::AABB& aabb, std::size_t capacity = 100, uint32_t depth = 0);
		SH_GAME_API Octree(Octree&& other) noexcept;
		SH_GAME_API ~Octree();

		SH_GAME_API void Clear();

		SH_GAME_API auto Query(const glm::vec3& pos) -> Octree*;
		SH_GAME_API auto Query(IOctreeElement& obj) -> std::vector<Octree*>;
		SH_GAME_API auto Query(const render::AABB& aabb) -> std::vector<IOctreeElement*>;
		SH_GAME_API void Query(const IOctreeElement& obj, std::vector<Octree*>& vec);
		SH_GAME_API void Query(const render::AABB& aabb, std::vector<IOctreeElement*>& vec);
		SH_GAME_API auto Insert(IOctreeElement& obj) -> bool;
		SH_GAME_API auto Erase(IOctreeElement& obj) -> bool;

		SH_GAME_API auto IsLeaf() const -> bool { return childs[0] == nullptr; }
		SH_GAME_API auto GetRoot() const -> Octree& { return *root; }
		SH_GAME_API auto GetElements() const -> const std::vector<IOctreeElement*>& { return objs; }
		SH_GAME_API auto GetElements() -> std::vector<IOctreeElement*>& { return objs; }
		SH_GAME_API auto GetBounds() const -> const render::AABB& { return aabb; }
	private:
		void Subdivide();
		bool InsertIntoChildren(IOctreeElement& obj);
	private:
		std::size_t capacity;
		uint32_t depth;
		uint32_t maxDepth = 8;

		render::AABB aabb;

		std::array<std::unique_ptr<Octree>, 8> childs;

		std::vector<IOctreeElement*> objs;

		Octree* root;
		Octree* parent = nullptr;
	};
}//namespace