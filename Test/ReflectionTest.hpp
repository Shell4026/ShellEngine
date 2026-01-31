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
private:
    PROPERTY(pr)
    int pr = 1;
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
    std::set<int> set;
    PROPERTY(ptr)
    int* ptr;
    PROPERTY(sobjectPtr)
    Derived* sobjectPtr;
    PROPERTY(sobjectArr)
    std::array<Derived*, 2> sobjectArr;
    PROPERTY(map)
    std::map<std::string, int> map;
    PROPERTY(constPtr)
    const int* constPtr;
    PROPERTY(constInt)
    const int constInt = 3;

    enum class Enum
    {
        Test1,
        Test2
    };
    PROPERTY(enumv);
    Enum enumv = Enum::Test2;
};

class Nested : public sh::core::SObject
{
    SCLASS(Nested)
public:
    PROPERTY(arr)
    std::vector<std::vector<int>> arr;
    PROPERTY(arr2)
    std::vector<std::vector<std::vector<int>>> arr2;
public:
    Nested()
    {
        for (int i = 0; i < 3; ++i)
        {
            std::vector<int> v{};
            for (int j = 0; j < 3; ++j)
            {
                v.push_back(i * 3 + j);
            }
            arr.push_back(std::move(v));
        }
        for (int i = 0; i < 3; ++i)
        {
            std::vector<std::vector<int>> v{};
            for (int j = 0; j < 3; ++j)
            {
                std::vector<int> v2;
                for (int k = 0; k < 3; ++k)
                {
                    v2.push_back(i * 9 + j * 3 + k);
                }
                v.push_back(std::move(v2));
            }
            arr2.push_back(std::move(v));
        }
    }
};

class FunctionObj : public sh::core::SObject
{
    SCLASS(FunctionObj)
public:
    FunctionObj() = default;

    SFUNCTION(SetValue)
    void SetValue(int v) { value = v; }
    SFUNCTION(GetValue)
    auto GetValue() const -> int { return value; }
    SFUNCTION(Sum)
    auto Sum(int a, int b) -> int { return a + b; }
    SFUNCTION(AddToValue)
    void AddToValue(int d) { value += d; }
    SFUNCTION(NoArgs)
    void NoArgs() { calledNoArgs = true; }
    SFUNCTION(WasNoArgsCalled)
    auto WasNoArgsCalled() const -> bool { return calledNoArgs; }

    SFUNCTION(StaticMul)
    static int StaticMul(int a, int b) { return a * b; }
private:
    int value = 0;
    bool calledNoArgs = false;
};

class FunctionDerived : public FunctionObj
{
    SCLASS(FunctionDerived)
public:
    SFUNCTION(Inc)
    int Inc(int x) { return x + 1; }
};

namespace A::B
{
    class C
    {
    public:
        class D
        {

        };
    };
}
struct TestStruct
{

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
    EXPECT_TRUE(*derived.GetType().super == Base::GetStaticType());

    EXPECT_EQ(base.GetType().type.size, sizeof(Base));
    EXPECT_EQ(derived.GetType().type.size, sizeof(Derived));

    using namespace sh::core;
    auto& type1 = reflection::GetType<int*>();
    auto& type2 = reflection::GetType<int>();
    auto& type3 = reflection::GetType<const int>();
    auto& type4 = reflection::GetType<int&>();
    auto& type5 = reflection::GetType<int*&>();
    EXPECT_FALSE(type1 == type2);
    EXPECT_FALSE(type2 == type3);
    EXPECT_FALSE(type3 == type4);
    EXPECT_FALSE(type4 == type5);
    EXPECT_FALSE(type5 == type1);
    // 해시는 다 같다.
    EXPECT_TRUE(type1.hash == type2.hash && type2.hash == type3.hash && type3.hash == type4.hash && type4.hash == type5.hash);

    // GetType으로 얻은 타입도 SCLASS라면 SType을 가져올 수 있다.
    EXPECT_TRUE(reflection::GetType<Base>().GetSType() == &Base::GetStaticType());
    // int는 SCLASS가 아님
    EXPECT_TRUE(reflection::GetType<int>().GetSType() == nullptr);
}
TEST(ReflectionTest, TypeNameTest)
{
    using namespace sh::core;
    {
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<int>() };
        EXPECT_EQ(typeName, "int");
        typeName = std::string{ reflection::TypeTraits::GetTypeName<int32_t>() };
        EXPECT_EQ(typeName, "int");
    }
    {
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<const int>() };
        EXPECT_EQ(typeName, "const int");
    }
    {
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<TestStruct>() };
        EXPECT_EQ(typeName, "TestStruct");
    }
    {
        struct A
        {
        };
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<A>() };
        // MSVC: struct ReflectionTest_TypeNameTest_Test::TestBody::A
        // GCC: ReflectionTest_TypeNameTest_Test::TestBody()::A
        EXPECT_NE(typeName, "A");
    }
    {
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<A::B::C>() };
        EXPECT_EQ(typeName, "A::B::C");
        typeName = std::string{ reflection::TypeTraits::GetTypeName<A::B::C::D>() };
        EXPECT_EQ(typeName, "A::B::C::D");
    }
    {
        using namespace A::B;
        std::string typeName = std::string{ reflection::TypeTraits::GetTypeName<C>() };
        EXPECT_EQ(typeName, "A::B::C");
    }
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

    EXPECT_EQ(*numberProperty->Get<int>(derived), 42);
    EXPECT_EQ(numbersProperty->type.name, sh::core::reflection::TypeTraits::GetTypeName<std::vector<int>>());
    EXPECT_TRUE(numbersProperty->isContainer);
    EXPECT_TRUE(numbersProperty->containerElementType != nullptr);
    EXPECT_EQ(*numbersProperty->containerElementType, sh::core::reflection::GetType<int>());
    numbersProperty->InsertToContainer(derived, 4);

    // enum
    auto* enumProp = derived.GetType().GetProperty("enumv");
    EXPECT_TRUE(enumProp->isEnum);
    EXPECT_EQ(*enumProp->Get<int>(derived), 1);
    *enumProp->Get<int>(derived) = 0;
    EXPECT_EQ(derived.enumv, Derived::Enum::Test1);

    auto begin = numbersProperty->Begin(derived);
    auto end = numbersProperty->End(derived);

    std::vector<int> expected = { 1, 2, 3, 4 };
    for (size_t i = 0; begin != end; ++begin, ++i) 
    {
        EXPECT_FALSE(begin.IsConst());
        EXPECT_EQ(*begin.Get<int>(), expected[i]);
    }

    // set테스트
    {
        for (int i = 0; i < 3; ++i)
            derived.set.insert(i);

        auto setProp = derived.GetType().GetProperty("set");
        EXPECT_TRUE(setProp->isContainer);
        //EXPECT_EQ(*setProp->containerElementType, sh::core::reflection::GetType<int>());
        //sh::core::reflection::GetContainerElementType<sh::core::SHashSet<int>>::type;
        auto setIterator = setProp->Begin(derived);
        EXPECT_TRUE(setIterator.IsConst());

        const int* ptr = setIterator.Get<const int>();
        EXPECT_EQ(*ptr, 0);
        ++setIterator;
        ptr = setIterator.Get<const int>();
        EXPECT_EQ(*ptr, 1);

        setProp->InsertToContainer(derived, 4);
        EXPECT_TRUE(derived.set.find(4) != derived.set.end());
    }

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
        auto arrIterator = prop->Begin(derived);
        EXPECT_FALSE(arrIterator.IsConst());
    }
    // map 요소 변경 테스트
    {
        derived.map.insert({ "test0", 0 });
        derived.map.insert({ "test1", 1 });
        auto prop = derived.GetType().GetProperty("map");
        EXPECT_TRUE(prop->isContainer);
        for (auto it = prop->Begin(derived); it != prop->End(derived); ++it)
        {
            EXPECT_TRUE(it.IsPair());
            EXPECT_TRUE(it.GetPairType()->first.isConst);
            EXPECT_TRUE(it.GetPairType()->first != sh::core::reflection::GetType<std::string>()); // map의 키값은 const기 때문에 다름
            EXPECT_TRUE(it.GetPairType()->first.hash == sh::core::reflection::GetType<std::string>().hash); // 해시는 같음
            EXPECT_TRUE(it.GetPairType()->second == sh::core::reflection::GetType<int>());
            auto pair = it.Get<std::pair<const std::string, int>>();
            pair->second++;
        }
        EXPECT_EQ(derived.map["test0"], 1);
        EXPECT_EQ(derived.map["test1"], 2);

        prop->InsertToContainer(derived, std::make_pair(std::string{ "test2" }, 3));
        EXPECT_EQ(derived.map["test2"], 3);
    }
}
TEST(ReflectionTest, ConstTest)
{
    Derived derived{};
    {
        derived.constPtr = new int(4);
        auto prop = derived.GetType().GetProperty("constPtr");
        EXPECT_FALSE(prop->isConst);
        const int* constIntegerPtr = *prop->Get<const int*>(derived);
        EXPECT_EQ(*constIntegerPtr, 4);
        int* integerPtr = *prop->Get<int*>(derived);
        EXPECT_EQ(*integerPtr, 4);
        *integerPtr = 5; // const인데도 바뀔 수 있다. 생성된 객체 자체는 수정 가능한 메모리 영역에 있기 때문
        delete derived.constPtr;
    }
    {
        auto prop = derived.GetType().GetProperty("constInt");
        EXPECT_TRUE(prop->isConst);
        const int* constIntegerPtr = prop->Get<const int>(derived);
        EXPECT_EQ(*constIntegerPtr, 3);
        // int* integerPtr = prop->Get<int>(&derived); // 디버그 모드에서 assert에 걸림
        int* integerPtr = prop->GetSafe<int>(derived);
        EXPECT_EQ(integerPtr, nullptr);
    }
}
TEST(ReflectionTest, NestedContainerPropertyTest)
{
    Nested nested0;
    auto prop = nested0.GetType().GetProperty("arr");
    EXPECT_EQ(prop->GetContainerNestedLevel(), 2);
    int expected = 0;

    sh::core::reflection::PropertyIteratorT savedIterator;
    for (auto it = prop->Begin(nested0); it != prop->End(nested0); ++it)
    {
        for (auto it2 = it.GetNestedBegin(); it2 != it.GetNestedEnd(); ++it2)
        {
            int* ptr = it2.Get<int>();
            if (*ptr == 4)
                savedIterator = it2;
            EXPECT_EQ(*ptr, expected++);
        }
    }
    EXPECT_EQ(*savedIterator.Get<int>(), 4);

    prop = nested0.GetType().GetProperty("arr2");
    EXPECT_EQ(prop->GetContainerNestedLevel(), 3);
    expected = 0;
    for (auto it = prop->Begin(nested0); it != prop->End(nested0); ++it)
    {
        for (auto it2 = it.GetNestedBegin(); it2 != it.GetNestedEnd(); ++it2)
        {
            for (auto it3 = it2.GetNestedBegin(); it3 != it2.GetNestedEnd(); ++it3)
            {
                int* ptr = it3.Get<int>();
                EXPECT_EQ(*ptr, expected++);
                if (expected == 27)
                    it3.Erase();
            }
        }
    }

    Nested nested1;
    nested1.arr.clear();
    prop = nested1.GetType().GetProperty("arr");
    auto itBegin = prop->Begin(nested1);
    auto itEnd = prop->End(nested1);
 
    EXPECT_TRUE(itBegin == itEnd);
}

TEST(ReflectionTest, SafePropertyTest)
{
    Derived derived;
    derived.number = 123;

    auto prop = Derived::GetStaticType().GetProperty("number");
    int number = *prop->GetSafe<int>(derived);
    EXPECT_EQ(number, 123);

    float* floatPtr = prop->GetSafe<float>(derived);
    EXPECT_EQ(floatPtr, nullptr);
}

TEST(ReflectionTest, FunctionRegistrationTest)
{
    auto& st = FunctionObj::GetStaticType();

    auto* f1 = st.GetFunction("SetValue");
    auto* f2 = st.GetFunction("GetValue");
    auto* f3 = st.GetFunction("Sum");
    auto* f4 = st.GetFunction("AddToValue");
    auto* f5 = st.GetFunction("NoArgs");

    ASSERT_NE(f1, nullptr);
    ASSERT_NE(f2, nullptr);
    ASSERT_NE(f3, nullptr);
    ASSERT_NE(f4, nullptr);
    ASSERT_NE(f5, nullptr);

    EXPECT_EQ(f1->GetName(), "SetValue");
    EXPECT_EQ(f2->GetName(), "GetValue");
}

TEST(ReflectionTest, FunctionInvokeTest)
{
    FunctionObj obj;

    // SetValue(int)
    {
        auto* fn = obj.GetType().GetFunction("SetValue");
        ASSERT_NE(fn, nullptr);

        int v = 123;
        fn->InvokeVoid(obj, v);
        EXPECT_EQ(obj.GetValue(), 123);
    }

    // AddToValue(int)
    {
        auto* fn = obj.GetType().GetFunction("AddToValue");
        ASSERT_NE(fn, nullptr);

        fn->InvokeVoid(obj, 7);
        EXPECT_EQ(obj.GetValue(), 130);
    }

    // Sum(int,int) -> int (반환값)
    {
        auto* fn = obj.GetType().GetFunction("Sum");
        ASSERT_NE(fn, nullptr);

        int a = 10, b = 20;
        int r = fn->Invoke<int>(obj, a, b);
        EXPECT_EQ(r, 30);
    }

    // NoArgs() / WasNoArgsCalled()
    {
        auto* noArgs = obj.GetType().GetFunction("NoArgs");
        auto* wasCalled = obj.GetType().GetFunction("WasNoArgsCalled");
        ASSERT_NE(noArgs, nullptr);
        ASSERT_NE(wasCalled, nullptr);

        EXPECT_FALSE(obj.WasNoArgsCalled());
        noArgs->InvokeVoid(obj);
        EXPECT_TRUE(obj.WasNoArgsCalled());

        // const object로 const 함수 호출 가능해야 함
        const FunctionObj& cref = obj;
        bool called = wasCalled->Invoke<bool>(cref);
        EXPECT_TRUE(called);
    }
}

TEST(ReflectionTest, FunctionInvokeRawTest)
{
    FunctionObj obj;

    // Sum을 Raw로 호출 (void** args, void* ret)
    auto* fn = obj.GetType().GetFunction("Sum");
    ASSERT_NE(fn, nullptr);

    int a = 3, b = 4;
    void* args[] = { &a, &b };
    int out = 0;

    fn->InvokeRaw(&obj, args, &out);
    EXPECT_EQ(out, 7);
}

TEST(ReflectionTest, FunctionStaticTest)
{
    auto* fn = FunctionObj::GetStaticType().GetFunction("StaticMul");
    ASSERT_NE(fn, nullptr);

    int a = 6, b = 7;
    int r = fn->Invoke<int>(*(FunctionObj*)nullptr, a, b);

    EXPECT_EQ(r, 42);
}

TEST(ReflectionTest, FunctionInheritanceLookupTest)
{
    FunctionDerived d;

    auto* inc = d.GetType().GetFunction("Inc");
    ASSERT_NE(inc, nullptr);

    int x = 9;
    int r = inc->Invoke<int>(d, x);
    EXPECT_EQ(r, 10);

    // 부모 함수도 자식 타입에서 GetFunction으로 찾아지는가
    auto* getValue = d.GetType().GetFunction("GetValue");
    ASSERT_NE(getValue, nullptr);

    // 부모쪽 SetValue도 동작해야 함
    auto* setValue = d.GetType().GetFunction("SetValue");
    ASSERT_NE(setValue, nullptr);

    int v = 55;
    setValue->InvokeVoid(d, v);
    int cur = getValue->Invoke<int>(d);
    EXPECT_EQ(cur, 55);
}