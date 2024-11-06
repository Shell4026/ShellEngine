#pragma once

#include "Game/Octree.h"
#include "Game/IOctreeElement.h"

#include <gtest/gtest.h>
#include <random>

TEST(OctreeTest, InsertTest)
{
	using namespace sh;
	class Element : public game::IOctreeElement
	{
	private:
		game::Vec3 pos;
		render::AABB aabb;
	public:
		Element(game::Vec3 pos):
			pos(pos),
			aabb(-1, -1, -1, 1, 1, 1)
		{
			aabb.Set(pos + aabb.GetMin(), pos + aabb.GetMax());
		}
		auto GetPos() const -> const game::Vec3& override
		{
			return pos;
		}
		bool Intersect(const render::AABB& aabb) const override
		{
			return aabb.Intersects(aabb);
		}
	};
	
	std::array<std::unique_ptr<Element>, 100> elements;
	std::random_device seed{};
	std::mt19937 device{ seed() };
	for (int i = 0; i < 100; ++i)
	{
		std::uniform_real_distribution<float> rnd(-100, 100);
		float x = rnd(device);
		float y = rnd(device);
		float z = rnd(device);
		elements[i] = std::make_unique<Element>(game::Vec3{ x, y, z });
	}

	game::Octree octree{ render::AABB{ -100, -100, -100, 100, 100, 100 }, 10 };
	for (int i = 0; i < 100; ++i)
	{
		bool check = octree.Insert(*elements[i]);
		EXPECT_TRUE(check);
	}
	auto list = octree.Query(*elements[0]);
}