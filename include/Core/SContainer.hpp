#pragma once
#include "Memory/SAllocator.hpp"
#include "GarbageCollection.h"

#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <optional>
#include <queue>
#include <assert.h>
#include <stdexcept>

namespace sh::core
{
	/// @brief 쓰레기 수집을 지원하는 std::vector와 동일한 역할을 하는 컨테이너.
	/// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 nullptr로 바뀐다.
	/// @brief [주의] 절대 std::vector의 다형성 용도로 사용하면 안 된다.
	/// @tparam T 타입
	template<typename T>
    class SVector : public std::vector<T>
    {
    public:
        SVector() :
            std::vector<T>()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SVector(Args&&... args) :
            std::vector<T>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        SVector(const SVector& other) :
            std::vector<T>(other)
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        SVector(SVector&& other) noexcept :
            std::vector<T>(std::move(other))
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->RemoveContainerTracking(&other);
        }
        ~SVector()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
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
    };
    /// @brief 쓰레기 수집을 지원하는 std::array와 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 nullptr로 바뀐다.
    /// @brief [주의] 절대 std::array의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    /// @tparam size 배열 사이즈
    template<typename T, std::size_t size>
    class SArray : public std::array<T, size>
    {
    public:
        SArray()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SArray(Args&&... args) :
            std::array<T, size>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        ~SArray()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
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
    };
    /// @brief 쓰레기 수집을 지원하는 std::set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 타입
    template<typename T>
    class SSet : public std::set<T>
    {
    public:
        SSet()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SSet(Args&&... args) :
            std::set<T>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        ~SSet()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
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
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_set과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_set의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T>
    class SHashSet : public std::unordered_set<T>
    {
    public:
        SHashSet()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SHashSet(Args&&... args) :
            std::unordered_set<T>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        ~SHashSet()
        {
            if constexpr (std::is_convertible_v<T, const SObject*>)
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
    };
    /// @brief 쓰레기 수집을 지원하는 std::map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U>
    class SMap : public std::map<T, U>
    {
    public:
        SMap()
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SMap(Args&&... args) :
            std::map<T, U>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        ~SMap()
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
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
    };
    /// @brief 쓰레기 수집을 지원하는 std::unordered_map과 동일한 역할을 하는 컨테이너.
    /// @brief SObject타입의 포인터면 쓰레기 수집 목록에 포함 되며 객체가 제거 되면 요소가 제거된다.
    /// @brief [주의] 절대 std::unordered_map의 다형성 용도로 사용하면 안 된다.
    /// @tparam T 키 타입
    /// @tparam U 값 타입
    template<typename T, typename U>
    class SHashMap : public std::unordered_map<T, U>
    {
    public:
        SHashMap()
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        template<class... Args>
        SHashMap(Args&&... args) :
            std::unordered_map<T, U>(std::forward<Args>(args)...)
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
                core::GarbageCollection::GetInstance()->AddContainerTracking(*this);
        }
        ~SHashMap()
        {
            if constexpr (std::is_convertible_v<T, const SObject*> || std::is_convertible_v<U, const SObject*>)
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
    };
}//namespace