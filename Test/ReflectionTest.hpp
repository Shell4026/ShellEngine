#pragma once

#include "../include/Core/SObject.h"
#include "../include/Core/Reflection.hpp"
#include "../include/Core/SContainer.hpp"

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
    PROPERTY(number, sh::core::PropertyOption::constant)
    int number;
    PROPERTY(numbers)
    std::vector<int> numbers;
    PROPERTY(set)
    sh::core::SSet<int> set;
    PROPERTY(ptr)
    int* ptr;
    PROPERTY(sobjectPtr)
    Derived* sobjectPtr;
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
        EXPECT_FALSE(begin.IsConst());
        EXPECT_EQ(*begin.Get<int>(), expected[i]);
    }

    // set테스트
    for(int i = 0; i < 3; ++i)
        derived.set.insert(i);
    
    auto setProp = derived.GetType().GetProperty("set");
    EXPECT_TRUE(setProp->isContainer);
    auto setIterator = setProp->Begin(&derived);
    EXPECT_TRUE(setIterator.IsConst());

    const int* ptr = setIterator.Get<const int>();
    EXPECT_EQ(*ptr, 0);

    // 포인터 테스트
    auto ptrProp = derived.GetType().GetProperty("ptr");
    EXPECT_TRUE(ptrProp->isPointer);
    EXPECT_FALSE(ptrProp->isSObjectPointer);
    ptrProp = derived.GetType().GetProperty("sobjectPtr");
    EXPECT_TRUE(ptrProp->isPointer);
    EXPECT_TRUE(ptrProp->isSObjectPointer);
}

TEST(ReflectionTest, SafePropertyTest)
{
    Derived derived;
    derived.number = 123;

    auto prop = Derived::GetStaticType().GetProperty("number");
    int number = *prop->GetSafe<int>(&derived);
    EXPECT_EQ(number, 123);

    float* floatPtr = prop->GetSafe<float>(&derived);
    EXPECT_EQ(floatPtr, nullptr);
}