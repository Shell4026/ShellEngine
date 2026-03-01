#pragma once

#include "Render/AABB.h"

#include <gtest/gtest.h>

TEST(AABBTest, ConstructorNormalizesSwappedBounds)
{
	sh::render::AABB aabb{ 10.0f, 2.0f, -1.0f, -5.0f, -4.0f, 3.0f };

	EXPECT_FLOAT_EQ(aabb.GetMin().x, -5.0f);
	EXPECT_FLOAT_EQ(aabb.GetMin().y, -4.0f);
	EXPECT_FLOAT_EQ(aabb.GetMin().z, -1.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().x, 10.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().y, 2.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().z, 3.0f);
}

TEST(AABBTest, SetNormalizesSwappedBounds)
{
	sh::render::AABB aabb{};
	aabb.Set({ 5.0f, 5.0f, 5.0f }, { -2.0f, -1.0f, -3.0f });

	EXPECT_FLOAT_EQ(aabb.GetMin().x, -2.0f);
	EXPECT_FLOAT_EQ(aabb.GetMin().y, -1.0f);
	EXPECT_FLOAT_EQ(aabb.GetMin().z, -3.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().x, 5.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().y, 5.0f);
	EXPECT_FLOAT_EQ(aabb.GetMax().z, 5.0f);
}

TEST(AABBTest, ExpandWithNegativeAmountKeepsValidBounds)
{
	sh::render::AABB aabb{ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
	aabb.Expand(-3.0f);

	EXPECT_LE(aabb.GetMin().x, aabb.GetMax().x);
	EXPECT_LE(aabb.GetMin().y, aabb.GetMax().y);
	EXPECT_LE(aabb.GetMin().z, aabb.GetMax().z);
}
