#pragma once
#ifdef Bool
#undef Bool
#endif

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
private:
    PROPERTY(privateNumber)
    int privateNumber = 88;
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
    PROPERTY(sobjectArr)
    std::array<Derived*, 2> sobjectArr;
    PROPERTY(map)
    sh::core::SMap<std::string, int> map;
};

TEST(ReflectionTest, TypeInfoTest) 
{
    using namespace sh::core;
    Base base;
    Derived derived;

    EXPECT_EQ(Base::GetStaticType().name, "Base");
    EXPECT_EQ(Derived::GetStaticType().name, "Derived");

    EXPECT_TRUE(derived.GetType() == Derived::GetStaticType());
    EXPECT_TRUE(derived.GetType().IsChildOf(Base::GetStaticType()));
    EXPECT_TRUE(*derived.GetType().GetSuper() == Base::GetStaticType());

    EXPECT_EQ(base.GetType().size, 72);
    EXPECT_EQ(derived.GetType().size, 208);

    auto& intType = reflection::GetType<const int>();
    EXPECT_EQ(reflection::GetType<int>(), reflection::GetType<const int>());
    EXPECT_TRUE(intType.isConst);
}

TEST(ReflectionTest, ContainerTest)
{
    using namespace sh::core;
    EXPECT_EQ(reflection::GetType<int>().containerNestedLevel, 0);
    EXPECT_EQ(reflection::GetType<std::vector<int>>().containerNestedLevel, 1);
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
    EXPECT_EQ(numbersProperty->type.name, sh::core::reflection::GetTypeName<std::vector<int>>());

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
    // Array 테스트
    {
        auto prop = derived.GetType().GetProperty("sobjectArr");
        EXPECT_EQ(prop->isContainer, true);
        auto arrIterator = prop->Begin(&derived);
        EXPECT_FALSE(arrIterator.IsConst());
    }
    // map 요소 변경 테스트
    {
        derived.map.insert({ "test0", 0 });
        derived.map.insert({ "test1", 1 });
        auto prop = derived.GetType().GetProperty("map");
        EXPECT_TRUE(prop->isContainer);
        for (auto it = prop->Begin(&derived); it != prop->End(&derived); ++it)
        {
            EXPECT_TRUE(it.IsPair());
            EXPECT_TRUE(it.GetPairType()->first.isConst);
            EXPECT_TRUE(it.GetPairType()->first == sh::core::reflection::GetType<std::string>());
            EXPECT_TRUE(it.GetPairType()->second == sh::core::reflection::GetType<int>());
            auto pair = it.Get<std::pair<const std::string, int>>();
            pair->second++;
        }
        EXPECT_EQ(derived.map["test0"], 1);
        EXPECT_EQ(derived.map["test1"], 2);
    }
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