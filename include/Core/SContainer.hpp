#pragma once
#include "SObject.h"
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
    class SVector : public std::vector<T>
    {
    public:
        SVector() :
            std::vector<T>()
        {
            AddToGC();
        }
        template<class... Args>
        SVector(Args&&... args) :
            std::vector<T>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SVector(const SVector& other) :
            std::vector<T>(other)
        {
            AddToGC();
        }
        SVector(SVector&& other) noexcept :
            std::vector<T>(std::move(other))
        {
            AddToGC();
        }
        ~SVector()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SVector& other) -> SVector&
        {
            std::vector<T>::operator=(other);
            return *this;
        }
        auto operator=(SVector&& other) noexcept -> SVector&
        {
            std::vector<T>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto& elem : *this)
                    {
                        const SObject* obj = static_cast<const SObject*>(elem);
                        if (obj == nullptr) continue;
                        if (obj->IsPendingKill()) { elem = nullptr; continue; }

                        bfs.push(const_cast<SObject*>(obj));
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::array와 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 nullptr로 바뀐다.
    /// @brief [주의] 절대 std::array의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    /// @tparam size 배열 사이즈
    template<typename T, std::size_t size, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SArray : public std::array<T, size>
    {
    public:
        SArray()
        {
            AddToGC();
        }
        template<class... Args>
        SArray(Args&&... args) :
            std::array<T, size>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SArray(const SArray& other) :
            std::array<T, size>(other)
        {
            AddToGC();
        }
        SArray(SArray&& other) noexcept :
            std::array<T, size>(std::move(other))
        {
            AddToGC();
        }
        ~SArray()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SArray& other) -> SArray&
        {
            std::array<T, size>::operator=(other);
            return *this;
        }
        auto operator=(SArray&& other) noexcept -> SArray&
        {
            std::array<T, size>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto& elem : *this)
                    {
                        const SObject* obj = static_cast<const SObject*>(elem);
                        if (obj == nullptr) continue;
                        if (obj->IsPendingKill()) { elem = nullptr; continue; }

                        bfs.push(const_cast<SObject*>(obj));
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SSet : public std::set<T>
    {
    public:
        SSet()
        {
            AddToGC();
        }
        template<class... Args>
        SSet(Args&&... args) :
            std::set<T>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SSet(const SSet& other) :
            std::set<T>(other)
        {
            AddToGC();
        }
        SSet(SSet&& other) noexcept:
            std::set<T>(std::move(other))
        {
            AddToGC();
        }
        ~SSet()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SSet& other) -> SSet&
        {
            std::set<T>::operator=(other);
            return *this;
        }
        auto operator=(SSet&& other) noexcept -> SSet&
        {
            std::set<T>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto it = begin(); it != end();)
                    {
                        const SObject* obj = static_cast<const SObject*>(*it);
                        if (obj == nullptr) { ++it; continue; }
                        if (obj->IsPendingKill()) { it = erase(it); continue; }

                        bfs.push(const_cast<SObject*>(obj));
                        ++it;
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SHashSet : public std::unordered_set<T>
    {
    public:
        SHashSet()
        {
            AddToGC();
        }
        template<class... Args>
        SHashSet(Args&&... args) :
            std::unordered_set<T>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SHashSet(const SHashSet& other) :
            std::unordered_set<T>(other)
        {
            AddToGC();
        }
        SHashSet(SHashSet&& other) noexcept :
            std::unordered_set<T>(std::move(other))
        {
            AddToGC();
        }
        ~SHashSet()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SHashSet& other) -> SHashSet&
        {
            std::unordered_set<T>::operator=(other);
            return *this;
        }
        auto operator=(SHashSet&& other) noexcept -> SHashSet&
        {
            std::unordered_set<T>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto it = begin(); it != end();)
                    {
                        const SObject* obj = static_cast<const SObject*>(*it);
                        if (obj == nullptr) { ++it; continue; }
                        if (obj->IsPendingKill()) { it = erase(it); continue; }

                        bfs.push(const_cast<SObject*>(obj));
                        ++it;
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>>>
    class SMap : public std::map<T, U>
    {
    public:
        SMap()
        {
            AddToGC();
        }
        template<class... Args>
        SMap(Args&&... args) :
            std::map<T, U>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SMap(const SMap& other) :
            std::map<T, U>(other)
        {
            AddToGC();
        }
        SMap(SMap&& other) noexcept :
            std::map<T, U>(std::move(other))
        {
            AddToGC();
        }
        ~SMap()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SMap& other) -> SMap&
        {
            std::map<T, U>::operator=(other);
            return *this;
        }
        auto operator=(SMap&& other) noexcept -> SMap&
        {
            std::map<T, U>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto it = begin(); it != end();)
                    {
                        const SObject* obj = nullptr;
                        if constexpr (std::is_convertible_v<T, const SObject*>)
                            obj = static_cast<const SObject*>(it->first);
                        else if constexpr (std::is_convertible_v<U, const SObject*>)
                            obj = static_cast<const SObject*>(it->second);

                        if (obj == nullptr) { ++it; continue; }
                        if (obj->IsPendingKill()) { it = erase(it); continue; }

                        bfs.push(const_cast<SObject*>(obj));
                        ++it;
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>>>
    class SHashMap : public std::unordered_map<T, U>
    {
    public:
        SHashMap()
        {
            AddToGC();
        }
        template<class... Args>
        SHashMap(Args&&... args) :
            std::unordered_map<T, U>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SHashMap(const SHashMap& other) :
            std::unordered_map<T, U>(other)
        {
            AddToGC();
        }
        SHashMap(SHashMap&& other) :
            std::unordered_map<T, U>(std::move(other))
        {
            AddToGC();
        }
        ~SHashMap()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SHashMap& other) -> SHashMap&
        {
            std::unordered_map<T, U>::operator=(other);
            return *this;
        }
        auto operator=(SHashMap&& other) noexcept -> SHashMap&
        {
            std::unordered_map<T, U>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto it = begin(); it != end();)
                    {
                        const SObject* obj = nullptr;
                        if constexpr (std::is_convertible_v<T, const SObject*>)
                            obj = static_cast<const SObject*>(it->first);
                        else if constexpr (std::is_convertible_v<U, const SObject*>)
                            obj = static_cast<const SObject*>(it->second);

                        if (obj == nullptr) { ++it; continue; }
                        if (obj->IsPendingKill()) { it = erase(it); continue; }

                        bfs.push(const_cast<SObject*>(obj));
                        ++it;
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
        }
    };

    /// @brief 쓰레기 수집을 지원하는 std::list와 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::list의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, const SObject*>>>
    class SList : public std::list<T>
    {
    public:
        SList()
        {
            AddToGC();
        }
        template<class... Args>
        SList(Args&&... args) :
            std::list<T>(std::forward<Args>(args)...)
        {
            AddToGC();
        }
        SList(const SList& other) :
            std::list<T>(other)
        {
            AddToGC();
        }
        SList(SList&& other) noexcept :
            std::list<T>(std::move(other))
        {
            AddToGC();
        }
        ~SList()
        {
            core::GarbageCollection::GetInstance()->RemoveContainerTracking(this);
        }
        auto operator=(const SList& other) -> SList&
        {
            std::list<T>::operator=(other);
            return *this;
        }
        auto operator=(SList&& other) noexcept -> SList&
        {
            std::list<T>::operator=(std::move(other));
            return *this;
        }
    private:
        void AddToGC()
        {
            core::GarbageCollection::TrackedContainer container{};
            container.ptr = this;
            container.markFn =
                [this](core::GarbageCollection& gc)
                {
                    std::queue<SObject*> bfs;

                    for (auto it = begin(); it != end();)
                    {
                        const SObject* obj = static_cast<const SObject*>(*it);
                        if (obj == nullptr) { ++it; continue; }
                        if (obj->IsPendingKill()) { it = erase(it); continue; }

                        bfs.push(const_cast<SObject*>(obj));
                        ++it;
                    }
                    gc.MarkBFS(bfs);
                };
            core::GarbageCollection::GetInstance()->AddContainerTracking(container);
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
            obj = other.obj;
            return *this;
        }
        auto operator=(SObjWeakPtr&& other) noexcept -> SObjWeakPtr&
        {
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
        auto operator->() -> T*
        {
            return obj;
        }
        auto operator->() const -> const T*
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