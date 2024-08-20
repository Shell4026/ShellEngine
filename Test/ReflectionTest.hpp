﻿#pragma once

#include "../include/Core/SObject.h"
#include "../include/Core/Reflection.hpp"

#include <gtest/gtest.h>

#include <string_view>

class Base : public sh::core::SObject 
{
    SCLASS(Base)
};

class Derived : public Base 
{
    SCLASS(Derived)
public:
    PROPERTY(number, "const")
        int number;
    PROPERTY(numbers)
        std::vector<int> numbers;
};

TEST(ReflectionTest, TypeInfoTest) 
{
    Base base;
    Derived derived;

    EXPECT_EQ(Base::GetStaticType().GetName(), "Base");
    EXPECT_EQ(Derived::GetStaticType().GetName(), "Derived");

    EXPECT_TRUE(derived.GetType().IsA(Derived::GetStaticType()));
    EXPECT_TRUE(derived.GetType().IsChildOf(Base::GetStaticType()));
}

TEST(ReflectionTest, PropertyTest) 
{
    Derived derived;
    derived.number = 42;
    derived.numbers = { 1, 2, 3 };

    auto* numberProperty = Derived::GetStaticType().GetProperty("number");
    auto* numbersProperty = Derived::GetStaticType().GetProperty("numbers");

    ASSERT_NE(numberProperty, nullptr);
    ASSERT_NE(numbersProperty, nullptr);

    EXPECT_EQ(*numberProperty->Get<int>(&derived), 42);
    EXPECT_EQ(numbersProperty->GetTypeName(), sh::core::reflection::GetTypeName<std::vector<int>>());

    auto begin = numbersProperty->Begin(&derived);
    auto end = numbersProperty->End(&derived);

    std::vector<int> expected = { 1, 2, 3 };
    for (size_t i = 0; begin != end; ++begin, ++i) 
    {
        EXPECT_EQ(*begin.Get<int>(), expected[i]);
    }
}