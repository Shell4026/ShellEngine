#include "Octree.h"
#include "IOctreeElement.h"

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
		childs(std::move(other.childs)), objs(std::move(other.objs))
	{

	}
	SH_GAME_API Octree::~Octree()
	{

	}

	void Octree::Subdivide()
	{
		if (childs[0])
			return;
		render::AABB childaabb[8];

		Vec3 mid = aabb.GetCenter();
		childaabb[0] = render::AABB{ aabb.GetMin(), mid};
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
	bool Octree::InsertIntoChildren(IOctreeElement& obj)
	{
		if (childs[0] == nullptr)
			return false;

		bool check = false;
		for (int i = 0; i < 8; ++i)
		{
			check |= childs[i]->Insert(obj);
		}
		return check;
	}

	void Octree::Query(const IOctreeElement& obj, std::vector<Octree*>& vec)
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
	void Octree::Query(const render::AABB& aabb, std::vector<IOctreeElement*>& vec)
	{
		if (IsLeaf())
		{
			if (aabb.Intersects(this->aabb))
			{
				for (auto element : objs)
				{
					if (element->Intersect(aabb))
						vec.push_back(element);
				}
			}
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				if (!aabb.Intersects(childs[i]->aabb))
					continue;
				childs[i]->Query(aabb, vec);
			}
		}
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

	SH_GAME_API bool Octree::Insert(IOctreeElement& obj)
	{
		if (!obj.Intersect(aabb))
		{
			if (root != this)
				return false;
			
			return false;
		}

		if (IsLeaf()) 
		{
			if (objs.size() < capacity) 
			{
				objs.insert(&obj);
				return true;
			}
			else 
			{
				if (depth == maxDepth)
					return false;

				Subdivide();
				for (IOctreeElement* elem : objs)
					InsertIntoChildren(*elem);
				objs.clear();
				return InsertIntoChildren(obj);
			}
		}
		return InsertIntoChildren(obj);
	}
	SH_GAME_API bool Octree::Erase(IOctreeElement& obj)
	{
		std::vector<Octree*> nodes{ Query(obj) };
		if (nodes.empty())
			return false;
		for(auto& node : nodes)
			node->objs.erase(&obj);

		return true;
	}

	SH_GAME_API bool Octree::IsLeaf() const
	{
		return childs[0] == nullptr;
	}

	SH_GAME_API auto Octree::GetRoot() const -> Octree&
	{
		return *root;
	}
	SH_GAME_API auto Octree::GetElements() const -> const std::unordered_set<IOctreeElement*>&
	{
		return objs;
	}
	SH_GAME_API auto Octree::GetElements() -> std::unordered_set<IOctreeElement*>&
	{
		return objs;
	}
	SH_GAME_API auto Octree::GetBounds() const -> const render::AABB&
	{
		return aabb;
	}
}