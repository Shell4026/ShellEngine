#include "PCH.h"
#include "Octree.h"

#include "GameObject.h"

#undef min
#undef max

namespace sh::game
{
	Octree::Octree(glm::vec3 min, glm::vec3 max, int capacity) :
		capacity(capacity), minSize(2), min(min), max(max), childs(), objs(), hasChild(false)
	{
	}
	Octree::Octree(Octree&& other) noexcept :
		capacity(other.capacity), minSize(other.minSize), min(other.min), max(other.max), 
		childs(std::move(other.childs)), objs(std::move(other.objs)), hasChild(other.hasChild)
	{

	}
	Octree::~Octree()
	{

	}

	auto Octree::Query(glm::vec3 pos) -> Octree*
	{
		if (min.x <= pos.x && min.y <= pos.y && min.z <= pos.z)
		{
			if (pos.x <= max.x && pos.y <= max.y && pos.z <= max.z)
			{
				if (!hasChild)
					return this;

				glm::vec3 center = (max + min) / 2.0f;
				
				if (pos.x < center.x) //왼쪽
				{
					if (pos.y < center.y) //위
					{
						if (pos.z < center.z)
							return childs[0]->Query(pos);
						else
							return childs[4]->Query(pos);
					}
					else //아래
					{
						if (pos.z < center.z)
							return childs[3]->Query(pos);
						else
							return childs[7]->Query(pos);
					}
				}
				if (pos.y < center.y) //오른쪽
				{
					if (pos.y < center.y) //위
					{
						if (pos.z < center.z)
							return childs[1]->Query(pos);
						else
							return childs[5]->Query(pos);
					}
					else //아래
					{
						if (pos.z < center.z)
							return childs[2]->Query(pos);
						else
							return childs[6]->Query(pos);
					}
				}
			}
		}
		return nullptr;
	}

	bool Octree::Insert(GameObject* obj)
	{
		if (!hasChild)
		{
			if (objs.size() + 1 > capacity)
			{
				glm::vec3 size = (max - min) / 2.0f;
				if (size.x < minSize || size.y < minSize || size.z < minSize)
					return false;

				hasChild = true;
				
				glm::vec3 center = (max + min) / 2.0f;
				
				childs[0] = std::make_unique<Octree>(
					glm::vec3{ center.x - size.x, center.y - size.y, center.z - size.z }, 
					glm::vec3{ center.x         , center.y         , center.z          }, capacity);
				childs[1] = std::make_unique<Octree>(
					glm::vec3{ center.x         , center.y - size.y, center.z - size.z },
					glm::vec3{ center.x + size.x, center.y         , center.z          }, capacity);
				childs[2] = std::make_unique<Octree>(
					glm::vec3{ center.x         , center.y         , center.z - size.z },
					glm::vec3{ center.x + size.x, center.y + size.y, center.z }, capacity);
				childs[3] = std::make_unique<Octree>(
					glm::vec3{ center.x - size.x, center.y         , center.z - size.z },
					glm::vec3{ center.x         , center.y + size.y, center.z          }, capacity);
				childs[4] = std::make_unique<Octree>(
					glm::vec3{ center.x - size.x, center.y - size.y, center.z          },
					glm::vec3{ center.x         , center.y         , center.z + size.z }, capacity);
				childs[5] = std::make_unique<Octree>(
					glm::vec3{ center.x         , center.y - size.y, center.z          },
					glm::vec3{ center.x + size.x, center.y         , center.z + size.z }, capacity);
				childs[6] = std::make_unique<Octree>(
					glm::vec3{ center.x         , center.y         , center.z          },
					glm::vec3{ center.x + size.x, center.y + size.y, center.z + size.z }, capacity);
				childs[7] = std::make_unique<Octree>(
					glm::vec3{ center.x - size.x, center.y         , center.z          },
					glm::vec3{ center.x         , center.y + size.y, center.z + size.z }, capacity);
			
				//기존에 있던 obj들을 자식에 재분배
				for (auto obj : objs)
				{
					Octree* child = Query(obj->transform->position);
					child->Insert(obj);
				}
				objs.clear();

				Octree* child = Query(obj->transform->position);
				if (child == nullptr)
					return false;
				return child->Insert(obj);
			}
			else
			{
				objs.push_back(obj);
				return true;
			}
		}
		else
		{
			Octree* child = Query(obj->transform->position);
			if (child == nullptr)
				return false;

			return child->Insert(obj);
		}
		return true;
	}

	auto Octree::GetObjs() const -> const std::vector<GameObject*>&
	{
		return objs;
	}
	auto Octree::GetObjs() -> std::vector<GameObject*>&
	{
		return objs;
	}
}