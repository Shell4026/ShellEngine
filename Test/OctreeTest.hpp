#pragma once

#include "Game/Octree.h"
#include "Game/IOctreeElement.h"

#include <gtest/gtest.h>
#include <algorithm>

namespace
{
	class TestElement : public sh::game::IOctreeElement
	{
	private:
		sh::game::Vec3 pos;
		sh::render::AABB aabb;
	public:
		TestElement(const sh::game::Vec3& pos, const sh::game::Vec3& halfExtents) :
			pos(pos),
			aabb(pos - halfExtents, pos + halfExtents)
		{
		}
		auto GetPos() const -> const sh::game::Vec3& override
		{
			return pos;
		}
		bool Intersect(const sh::render::AABB& other) const override
		{
			return aabb.Intersects(other);
		}
	};
}

TEST(OctreeTest, KeepStraddlingObjectInCurrentNode)
{
	sh::game::Octree octree{ sh::render::AABB{ -100, -100, -100, 100, 100, 100 }, 1 };
	TestElement childOnly{ sh::game::Vec3{ -60, -60, -60 }, sh::game::Vec3{ 5, 5, 5 } };
	TestElement straddling{ sh::game::Vec3{ 0, 0, 0 }, sh::game::Vec3{ 30, 30, 30 } };

	EXPECT_TRUE(octree.Insert(childOnly));
	EXPECT_TRUE(octree.Insert(straddling));

	const auto& rootElements = octree.GetElements();
	ASSERT_EQ(rootElements.size(), 1);
	EXPECT_EQ(rootElements[0], &straddling);
}

TEST(OctreeTest, QueryIncludesObjectsStoredInInternalNode)
{
	sh::game::Octree octree{ sh::render::AABB{ -100, -100, -100, 100, 100, 100 }, 1 };
	TestElement childOnly{ sh::game::Vec3{ -60, -60, -60 }, sh::game::Vec3{ 5, 5, 5 } };
	TestElement straddling{ sh::game::Vec3{ 0, 0, 0 }, sh::game::Vec3{ 30, 30, 30 } };

	EXPECT_TRUE(octree.Insert(childOnly));
	EXPECT_TRUE(octree.Insert(straddling));

	auto queried = octree.Query(sh::render::AABB{ -10, -10, -10, 10, 10, 10 });
	EXPECT_TRUE(std::find(queried.begin(), queried.end(), &straddling) != queried.end());
}

TEST(OctreeTest, InsertSameObjectTwiceDoesNotDuplicate)
{
	sh::game::Octree octree{ sh::render::AABB{ -100, -100, -100, 100, 100, 100 }, 10 };
	TestElement element{ sh::game::Vec3{ 10, 10, 10 }, sh::game::Vec3{ 1, 1, 1 } };

	EXPECT_TRUE(octree.Insert(element));
	EXPECT_TRUE(octree.Insert(element));

	const auto& elements = octree.GetElements();
	ASSERT_EQ(elements.size(), 1);
	EXPECT_EQ(elements[0], &element);
}

TEST(OctreeTest, EraseRemovesObjectFromTree)
{
	sh::game::Octree octree{ sh::render::AABB{ -100, -100, -100, 100, 100, 100 }, 1 };
	TestElement childOnly{ sh::game::Vec3{ -60, -60, -60 }, sh::game::Vec3{ 5, 5, 5 } };
	TestElement straddling{ sh::game::Vec3{ 0, 0, 0 }, sh::game::Vec3{ 30, 30, 30 } };

	EXPECT_TRUE(octree.Insert(childOnly));
	EXPECT_TRUE(octree.Insert(straddling));
	EXPECT_TRUE(octree.Erase(straddling));
	EXPECT_FALSE(octree.Erase(straddling));

	auto queried = octree.Query(sh::render::AABB{ -10, -10, -10, 10, 10, 10 });
	EXPECT_TRUE(std::find(queried.begin(), queried.end(), &straddling) == queried.end());
}
