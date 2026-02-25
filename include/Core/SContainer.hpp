#pragma once
#include "SObject.h"
#include "GCObject.h"
#include "GarbageCollection.h"
#include "Reflection/TypeTraits.hpp"

#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <list>
#include <optional>
#include <queue>
#include <assert.h>
#include <stdexcept>

namespace sh::core
{
	/// @brief 쓰레기 수집을 지원하는 std::vector와 동일한 역할을 하는 컨테이너.
	/// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 nullptr로 바뀐다.
	/// @brief [주의] 절대 std::vector의 다형성 용도로 사용하면 안 된다.
	/// @tparam SObject* 타입
	template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SVector : public std::vector<T>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto& elem : *this)
                gc.PushReferenceObject(elem);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::array와 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 nullptr로 바뀐다.
    /// @brief [주의] 절대 std::array의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    /// @tparam size 배열 사이즈
    template<typename T, std::size_t size, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SArray : public std::array<T, size>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto& elem : *this)
                gc.PushReferenceObject(elem);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SSet : public std::set<T>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto it = this->begin(); it != this->end();)
            {
                T elem = *it;
                if (elem->IsPendingKill())
                {
                    it = this->erase(it);
                    continue;
                }
                gc.PushReferenceObject(elem);
                ++it;
            }
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SHashSet : public std::unordered_set<T>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto it = this->begin(); it != this->end();)
            {
                T elem = *it;
                if (elem->IsPendingKill())
                {
                    it = this->erase(it);
                    continue;
                }
                gc.PushReferenceObject(elem);
                ++it;
            }
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>>>
    class SMap : public std::map<T, U>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto it = this->begin(); it != this->end();)
            {
                if constexpr (std::is_convertible_v<T, const SObject*>)
                {
                    T elem = it->first;
                    if (elem->IsPendingKill())
                    {
                        it = this->erase(it);
                        continue;
                    }
                    gc.PushReferenceObject(elem);
                }
                else
                {
                    U elem = it->second;
                    if (elem->IsPendingKill())
                    {
                        it = this->erase(it);
                        continue;
                    }
                    gc.PushReferenceObject(elem);
                }
                ++it;
            }
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>>>
    class SHashMap : public std::unordered_map<T, U>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto it = this->begin(); it != this->end();)
            {
                if constexpr (std::is_convertible_v<T, const SObject*>)
                {
                    T elem = it->first;
                    if (elem->IsPendingKill())
                    {
                        it = this->erase(it);
                        continue;
                    }
                    gc.PushReferenceObject(elem);
                }
                else
                {
                    U elem = it->second;
                    if (elem->IsPendingKill())
                    {
                        it = this->erase(it);
                        continue;
                    }
                    gc.PushReferenceObject(elem);
                }
                ++it;
            }
        }
    };

    /// @brief 쓰레기 수집을 지원하는 std::list와 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::list의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SList : public std::list<T>, public GCObject
    {
    public:
        void PushReferenceObjects(GarbageCollection& gc) override
        {
            for (auto it = this->begin(); it != this->end();)
            {
                T elem = *it;
                if (elem->IsPendingKill())
                {
                    it = this->erase(it);
                    continue;
                }
                gc.PushReferenceObject(elem);
                ++it;
            }
        }
    };

    /// @brief GC에 추적 되는 포인터 객체. 가르키는 객체가 소멸되면 nullptr로 바뀐다.
    /// @brief 해당 포인터로 객체를 참조 하고 있어도 GC에서 마킹을 하진 않는다.
    /// @tparam T SObject타입
    template<typename T, typename IsSObject>
    class SObjWeakPtr
    {
    private:
        T* obj = nullptr;
    public:
        SObjWeakPtr()
        {
            core::GarbageCollection::GetInstance()->AddPointerTracking(*this);
        }
        SObjWeakPtr(T* obj) :
            obj(obj)
        {
            core::GarbageCollection::GetInstance()->AddPointerTracking(*this);
        }
        SObjWeakPtr(const SObjWeakPtr& other) noexcept :
            obj(other.obj)
        {
            core::GarbageCollection::GetInstance()->AddPointerTracking(*this);
        }
        SObjWeakPtr(SObjWeakPtr&& other) noexcept :
            obj(other.obj)
        {
            core::GarbageCollection::GetInstance()->AddPointerTracking(*this);
            other.obj = nullptr;
        }
        ~SObjWeakPtr()
        {
            core::GarbageCollection::GetInstance()->RemovePointerTracking(*this);
        }
        auto operator=(const SObjWeakPtr& other) noexcept -> SObjWeakPtr&
        {
            if (&other == this)
                return *this;
            obj = other.obj;
            return *this;
        }
        auto operator=(SObjWeakPtr&& other) noexcept -> SObjWeakPtr&
        {
            if (&other == this)
                return *this;
            obj = other.obj;
            other.obj = nullptr;
            return *this;
        }
        auto operator=(T* obj) -> SObjWeakPtr&
        {
            this->obj = obj;
            return *this;
        }
        auto operator==(T* obj) const -> bool
        {
            return this->obj == obj;
        }
        auto operator!=(T* obj) const -> bool
        {
            return this->obj != obj;
        }
        auto operator==(const SObjWeakPtr& other) const -> bool
        {
            return obj == other.obj;
        }
        auto operator!=(const SObjWeakPtr& other) const -> bool
        {
            return obj != other.obj;
        }
        auto operator->() const -> T*
        {
            return obj;
        }
        auto operator*() const -> T&
        {
            return *obj;
        }
        void Reset()
        {
            obj = nullptr;
        }
        auto Get() -> T*
        {
            return obj;
        }
        auto Get() const -> T*
        {
            return obj;
        }
        auto IsValid() const -> bool
        {
            return core::IsValid(obj);
        }
    };
}//namespace

namespace sh::core::reflection
{
    template<typename T, typename Check>
    struct IsVector<SVector<T, Check>> : std::bool_constant<true> {};
    template<typename T, typename U, typename Check>
    struct IsMap<SMap<T, U, Check>, void> : std::bool_constant<true> {};
    template<typename... Args>
    struct IsHashMap<SHashMap<Args...>> : std::bool_constant<true> {};
    template<typename... Args>
    struct IsSet<SSet<Args...>> : std::bool_constant<true> {};
    template<typename... Args>
    struct IsHashSet<SHashSet<Args...>> : std::bool_constant<true> {};
    template<typename T, typename Check>
    struct IsList<SList<T, Check>> : std::bool_constant<true> {};

    template<typename T, typename Check>
    struct GetContainerNestedCount<SVector<T, Check>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
    template<typename T, typename U, typename Check>
    struct GetContainerNestedCount<SMap<T, U, Check>> : std::integral_constant<uint32_t, GetContainerNestedCount<U>::value + 1> {};
    template<typename T, typename U, typename Check>
    struct GetContainerNestedCount<SHashMap<T, U, Check>> : std::integral_constant<uint32_t, GetContainerNestedCount<U>::value + 1> {};
    template<typename T, typename Check>
    struct GetContainerNestedCount<SSet<T, Check>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
    template<typename T, typename Check>
    struct GetContainerNestedCount<SHashSet<T, Check>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};

    template<typename T, typename Check>
    struct GetContainerElementType<SVector<T, Check>>
    {
        using type = T;
    };
    template<typename T, typename U, typename Check>
    struct GetContainerElementType<SMap<T, U, Check>>
    {
        using type = std::pair<T, U>;
    };
    template<typename T, typename U, typename Check>
    struct GetContainerElementType<SHashMap<T, U, Check>>
    {
        using type = std::pair<T, U>;
    };
    template<typename T, typename Check>
    struct GetContainerElementType<SSet<T, Check>>
    {
        using type = T;
    };
    template<typename T, typename Check>
    struct GetContainerElementType<SHashSet<T, Check>>
    {
        using type = T;
    };
    template<typename T, typename Check>
    struct GetContainerElementType<SList<T, Check>>
    {
        using type = T;
    };

    template<typename T, typename Check>
    struct GetContainerLastType<SVector<T, Check>>
    {
        using type = typename GetContainerLastType<T>::type;
    };
    template<typename T, typename U, typename Check>
    struct GetContainerLastType<SMap<T, U, Check>>
    {
        using type = typename GetContainerLastType<U>::type;
    };
    template<typename T, typename U, typename Check>
    struct GetContainerLastType<SHashMap<T, U, Check>>
    {
        using type = typename GetContainerLastType<U>::type;
    };
    template<typename T, typename Check>
    struct GetContainerLastType<SSet<T, Check>>
    {
        using type = typename GetContainerLastType<T>::type;
    };
    template<typename T, typename Check>
    struct GetContainerLastType<SHashSet<T, Check>>
    {
        using type = typename GetContainerLastType<T>::type;
    };
}//namespace