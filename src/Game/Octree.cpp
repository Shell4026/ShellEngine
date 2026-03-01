#include "Octree.h"
#include "IOctreeElement.h"
#include <algorithm>

#undef min
#undef max

namespace sh::game
{
	SH_GAME_API Octree::Octree(const render::AABB& aabb, std::size_t capacity, uint32_t depth) :
		capacity(capacity), depth(depth),
		aabb(aabb), root(this)
	{
	}
	SH_GAME_API Octree::Octree(Octree&& other) noexcept :
		capacity(other.capacity), depth(other.depth),
		aabb(std::move(other.aabb)),
		childs(std::move(other.childs)), objs(std::move(other.objs)),
		root(other.root == &other ? this : other.root),
		parent(other.parent),
		maxDepth(other.maxDepth)
	{
		const auto fixTreeLinks = [&](auto&& self, Octree& node, Octree* treeRoot, Octree* treeParent) -> void
		{
			node.root = treeRoot;
			node.parent = treeParent;
			for (auto& child : node.childs)
			{
				if (!child)
					continue;
				self(self, *child, treeRoot, &node);
			}
		};
		for (auto& child : childs)
		{
			if (!child)
				continue;
			fixTreeLinks(fixTreeLinks, *child, root, this);
		}
	}
	SH_GAME_API Octree::~Octree()
	{

	}

	SH_GAME_API void Octree::Clear()
	{
		for (auto& child : childs)
			child.reset();
		objs.clear();
	}
	SH_GAME_API auto Octree::Query(const glm::vec3& pos) -> Octree*
	{
		if (IsLeaf())
		{
			if (aabb.Contains(pos))
				return this;
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				if (!childs[i]->aabb.Contains(pos))
					continue;
				return childs[i]->Query(pos);
			}
		}
		return nullptr;
	}
	SH_GAME_API auto Octree::Query(IOctreeElement& obj) -> std::vector<Octree*>
	{
		std::vector<Octree*> resultVec{};
		Query(obj, resultVec);
		return resultVec;
	}
	SH_GAME_API auto Octree::Query(const render::AABB& aabb) -> std::vector<IOctreeElement*>
	{
		std::vector<IOctreeElement*> resultVec{};
		Query(aabb, resultVec);
		return resultVec;
	}
	SH_GAME_API void Octree::Query(const IOctreeElement& obj, std::vector<Octree*>& vec)
	{
		if (IsLeaf())
		{
			if (obj.Intersect(aabb))
				return vec.push_back(this);
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				if (!obj.Intersect(childs[i]->aabb))
					continue;
				childs[i]->Query(obj, vec);
			}
		}
	}
	SH_GAME_API void Octree::Query(const render::AABB& aabb, std::vector<IOctreeElement*>& vec)
	{
		if (!aabb.Intersects(this->aabb))
			return;

		for (auto element : objs)
		{
			if (element->Intersect(aabb))
				vec.push_back(element);
		}

		if (!IsLeaf())
		{
			for (int i = 0; i < 8; ++i)
			{
				if (!aabb.Intersects(childs[i]->aabb))
					continue;
				childs[i]->Query(aabb, vec);
			}
		}
	}

	SH_GAME_API auto Octree::Insert(IOctreeElement& obj) -> bool
	{
		if (!obj.Intersect(aabb))
			return false;

		if (IsLeaf()) 
		{
			if (objs.size() < capacity) 
			{
				if (std::find(objs.begin(), objs.end(), &obj) == objs.end())
					objs.push_back(&obj);
				return true;
			}
			else 
			{
				if (depth == maxDepth)
				{
					if (std::find(objs.begin(), objs.end(), &obj) == objs.end())
						objs.push_back(&obj);
					return true;
				}

				Subdivide();
				std::vector<IOctreeElement*> prevObjs{};
				prevObjs.swap(objs);
				for (IOctreeElement* elem : prevObjs)
				{
					if (!InsertIntoChildren(*elem))
						objs.push_back(elem);
				}
				if (InsertIntoChildren(obj))
					return true;
				if (std::find(objs.begin(), objs.end(), &obj) == objs.end())
					objs.push_back(&obj);
				return true;
			}
		}
		if (InsertIntoChildren(obj))
			return true;
		if (std::find(objs.begin(), objs.end(), &obj) == objs.end())
			objs.push_back(&obj);
		return true;
	}
	SH_GAME_API bool Octree::Erase(IOctreeElement& obj)
	{
		const auto eraseRecursive = [&](auto&& self, Octree& node) -> bool
		{
			const auto prevSize = node.objs.size();
			node.objs.erase(
				std::remove(node.objs.begin(), node.objs.end(), &obj),
				node.objs.end()
			);
			bool erased = node.objs.size() != prevSize;
			for (auto& child : node.childs)
			{
				if (!child)
					continue;
				erased |= self(self, *child);
			}
			return erased;
		};
		return eraseRecursive(eraseRecursive, GetRoot());
	}

	void Octree::Subdivide()
	{
		if (childs[0])
			return;
		render::AABB childaabb[8];

		Vec3 mid = aabb.GetCenter();
		childaabb[0] = render::AABB{ aabb.GetMin(), mid };
		childaabb[1] = render::AABB{ Vec3{ mid.x, aabb.GetMin().y, aabb.GetMin().z }, Vec3{ aabb.GetMax().x, mid.y, mid.z } };
		childaabb[2] = render::AABB{ Vec3{ aabb.GetMin().x, mid.y, aabb.GetMin().z }, Vec3{ mid.x, aabb.GetMax().y, mid.z } };
		childaabb[3] = render::AABB{ Vec3{ mid.x, mid.y, aabb.GetMin().z }, Vec3{ aabb.GetMax().x, aabb.GetMax().y, mid.z } };
		childaabb[4] = render::AABB{ Vec3{ aabb.GetMin().x, aabb.GetMin().y, mid.z }, Vec3{ mid.x, mid.y, aabb.GetMax().z } };
		childaabb[5] = render::AABB{ Vec3{ mid.x, aabb.GetMin().y, mid.z }, Vec3{ aabb.GetMax().x, mid.y, aabb.GetMax().z } };
		childaabb[6] = render::AABB{ Vec3{ aabb.GetMin().x, mid.y, mid.z }, Vec3{ mid.x, aabb.GetMax().y, aabb.GetMax().z } };
		childaabb[7] = render::AABB{ mid, aabb.GetMax() };

		for (int i = 0; i < 8; ++i)
		{
			childs[i] = std::make_unique<Octree>(childaabb[i], this->capacity, depth + 1);
			childs[i]->parent = this;
			childs[i]->root = root;
		}
	}
	auto Octree::InsertIntoChildren(IOctreeElement& obj) -> bool
	{
		if (childs[0] == nullptr)
			return false;

		int intersectCount = 0;
		int childIdx = -1;
		for (int i = 0; i < 8; ++i)
		{
			if (!obj.Intersect(childs[i]->aabb))
				continue;
			++intersectCount;
			childIdx = i;
			if (intersectCount > 1)
				return false;
		}
		if (intersectCount != 1)
			return false;
		return childs[childIdx]->Insert(obj);
	}
}//namespace