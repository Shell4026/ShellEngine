﻿#pragma once

#include "Export.h"
#include "Vector.h"

#include "Core/SContainer.hpp"

#include "Render/AABB.h"

#include <array>
#include <memory>

namespace sh::game
{
	class IOctreeElement;

	class Octree
	{
	private:
		std::size_t capacity;
		uint32_t depth;
		uint32_t maxDepth = 8;

		render::AABB aabb;

		std::array<std::unique_ptr<Octree>, 8> childs;

		core::SSet<IOctreeElement*> objs;

		Octree* root;
		Octree* parent = nullptr;
	private:
		void Subdivide();
		bool InsertIntoChildren(IOctreeElement& obj);
		void Query(const IOctreeElement& obj, core::SVector<Octree*>& vec);
		void Query(const render::AABB& aabb, core::SVector<IOctreeElement*>& vec);
	public:
		SH_GAME_API Octree(const render::AABB& aabb, std::size_t capacity = 100, uint32_t depth = 0);
		SH_GAME_API Octree(Octree&& other) noexcept;
		SH_GAME_API ~Octree();

		SH_GAME_API auto Query(const glm::vec3& pos) -> Octree*;
		SH_GAME_API auto Query(IOctreeElement& obj) -> core::SVector<Octree*>;
		SH_GAME_API auto Query(const render::AABB& aabb) -> core::SVector<IOctreeElement*>;
		SH_GAME_API bool Insert(IOctreeElement& obj);
		SH_GAME_API bool Erase(IOctreeElement& obj);

		SH_GAME_API bool IsLeaf() const;

		SH_GAME_API auto GetRoot() const -> Octree&;
		SH_GAME_API auto GetElements() const -> const core::SSet<IOctreeElement*>&;
		SH_GAME_API auto GetElements() -> core::SSet<IOctreeElement*>&;
		SH_GAME_API auto GetBounds() const -> const render::AABB&;
	};
}