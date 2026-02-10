# 개요
리플렉션 시스템이란, 런타임 중에 타입의 정보를 가져올 수 있는 시스템을 말합니다.</br>
엔진의 핵심적인 요소로 이를 이용해 에디터, 가비지 컬렉터, 직렬화 기능을 구현했습니다.

# 사용법
SCLASS(클래스 이름) 매크로를 클래스에 선언하면 해당 클래스는 리플렉션 정보를 갖게 됩니다.
```c++
class Derived : public Base 
{
    SCLASS(Derived)
}
```
일반 변수에 대한 타입도 가져올 수 있으나 SCLASS보단 정보가 한정적입니다.
```c++
using namespace sh::core; // sh::core::reflection...

const reflection::TypeInfo& intType = reflection::GetType<int>();
assert(intType.size == sizeof(int));
```
해당 클래스에 대한 정보는 GetStaticType()으로 가져올 수 있습니다.
```c++
reflection::STypeInfo& derivedType = Derived::GetStaticType();

assert(derivedType.name == "Derived");
assert(derivedType.type.isPointer == false);

// GetType으로 얻은 타입도 SCLASS라면 SType을 가져올 수 있다.
assert(reflection::GetType<Derived>().GetSType() == &derivedType);
// int는 SCLASS가 아님
assert(reflection::GetType<int>().GetSType() == nullptr);
```
인스턴스화 된 객체의 정보는 GetType()으로도 가져올 수 있습니다.
```c++
Derived derived;
assert(derived.GetType() == Derived::GetStaticType());
assert(derived.GetType().IsChildOf(Base::GetStaticType())); // Base의 자식 객체다.
assert(*derived.GetType().super == Base::GetStaticType()); // derived인스턴스의 부모 클래스는 Base다.
```
클래스 내 변수를 리플렉션에 노출 시키려면 PROPERTY()매크로를 통해 알려줘야합니다.</br>
리플렉션에 노출 된 SObject타입의 포인터 프로퍼티는 가비지 컬렉터에서 추적됩니다.</br>
```c++
PROPERTY(number, sh::core::PropertyOption::constant)
int number;
PROPERTY(numbers)
std::vector<int> numbers;
/*
...
*/
Derived derived;
derived.number = 42;
derived.numbers = { 1, 2, 3 };

using namespace sh;
core::reflection::Property* numberProperty = Derived::GetStaticType().GetProperty("number");
core::reflection::Property* numbersProperty = Derived::GetStaticType().GetProperty("numbers");

int* intPtr = numberProperty->Get<int>(&derived); // 타입 검사는 하지 않는다. 검사가 필요하면 GetSafe()함수 사용
assert(*intPtr == 42);
assert(numbersProperty->type.name == core::reflection::GetTypeName<std::vector<int>>());
*intPtr = 27; // 값 변경
assert(dereived.number == 27);
```
전방 선언을 한 SObject타입의 포인터가 있다면 컴파일 시간에 타입을 추론 할 수 없어 명시적으로 프로퍼티 옵션에 SObject라고 알려줘야 합니다.</br>
```c++
class ThisIsSObject;
...
PROPERTY(ptr, sh::core::PropertyOption::sobjPtr)
ThisIsSObject* ptr = nullptr;
```
컨테이너 프로퍼티는 반복자를 통해 순회 할 수 있습니다.
```c++
auto begin = numbersProperty->Begin(&derived);
auto end = numbersProperty->End(&derived);

std::vector<int> expected = { 1, 2, 3 };
for (size_t i = 0; begin != end; ++begin, ++i) 
{
    assert(!begin.IsConst());
    assert(*begin.Get<int>() == expected[i]);
}
```
중첩 컨테이너 역시 처리 할 수 있습니다.
```c++
PROPERTY(arr)
std::vector<std::vector<int>> arr;
/*
...
*/
Nested nested;
auto prop = nested.GetType().GetProperty("arr");
assert(prop->GetContainerNestedLevel() == 2); // 컨테이너 중첩 수

using namespace sh;
core::reflection::PropertyIterator savedIterator;
int expected = 0;
for (auto it = prop->Begin(&nested); it != prop->End(&nested); ++it)
{
    for (auto it2 = it.GetNestedBegin(); it2 != it.GetNestedEnd(); ++it2)
    {
        int* ptr = it2.Get<int>();
        if (*ptr == 4)
            savedIterator = it2;
        assert(*ptr == expected++);
    }
}
assert(*savedIterator.Get<int>() == 4); // 반복자 저장 가능
```

컨테이너의 종류에 상관없이 값을 넣을 수도 있습니다.
```c++
// std::vector<int> numbers{ 1, 2, 3 };
auto* numbersProperty = Derived::GetStaticType().GetProperty("numbers");
assert(numbersProperty->isContainer);
assert(*numbersProperty->containerElementType == sh::core::reflection::GetType<int>());
numbersProperty->InsertToContainer(derived, 4);
assert(numbers[3] == 4);

// std::map<std::string, int> map{};
auto mapProp = derived.GetType().GetProperty("map");
mapProp->InsertToContainer(derived, std::make_pair(std::string{ "test2" }, 3)); // "test2"로 넣으면 에러
```
