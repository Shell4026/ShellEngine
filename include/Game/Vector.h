#pragma once

#include "Export.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "reactphysics3d/mathematics/Vector2.h"
#include "reactphysics3d/mathematics/Vector3.h"

#include <cassert>
#include <type_traits>

namespace sh::game
{
    template <typename, typename = void>
    struct HasX : std::false_type {};
    template <typename T>
    struct HasX<T, std::void_t<decltype(std::declval<T>().x)>> : std::true_type {};

    template <typename, typename = void>
    struct HasY : std::false_type {};
    template <typename T>
    struct HasY<T, std::void_t<decltype(std::declval<T>().y)>> : std::true_type {};

    template <typename, typename = void>
    struct HasZ : std::false_type {};
    template <typename T>
    struct HasZ<T, std::void_t<decltype(std::declval<T>().z)>> : std::true_type {};

    template <typename, typename = void>
    struct HasW : std::false_type {};
    template <typename T>
    struct HasW<T, std::void_t<decltype(std::declval<T>().w)>> : std::true_type {};
    
    struct Empty {};

    template<std::size_t N>
    struct Vec
    {
        union 
        {
            float data[N];
            struct
            {
                std::conditional_t<N >= 1, float, Empty> x;
                std::conditional_t<N >= 2, float, Empty> y;
                std::conditional_t<N >= 3, float, Empty> z;
                std::conditional_t<N >= 4, float, Empty> w;
            };
        };

        operator glm::vec2() const
        {
            static_assert(N >= 1);

            if constexpr (N >= 2)
                return glm::vec2{ x, y };
            else if constexpr (N >= 1)
                return glm::vec2{ x, 0.f };
        }
        operator glm::vec3() const
        {
            static_assert(N >= 1);

            if constexpr (N >= 3)
                return glm::vec3{ x, y, z };
            else if constexpr (N >= 2)
                return glm::vec3{ x, y, 0.f };
            else if constexpr (N >= 1)
                return glm::vec3{ x, 0.f, 0.f };
        }
        operator glm::vec4() const
        {
            static_assert(N >= 1);

            if constexpr (N >= 4)
                return glm::vec4{ x, y, z, w };
            else if constexpr (N >= 3)
                return glm::vec4{ x, y, z, 0.f };
            else if constexpr (N >= 2)
                return glm::vec4{ x, y, 0.f, 0.f };
            else if constexpr (N >= 1)
                return glm::vec4{ x, 0.f, 0.f, 0.f };
        }
        operator reactphysics3d::Vector2() const
        {
            static_assert(N >= 1);

            if constexpr (N >= 2)
                return reactphysics3d::Vector2{ x, y };
            else if constexpr (N >= 1)
                return greactphysics3d::Vector2{ x, 0.f };
        }
        operator reactphysics3d::Vector3() const
        {
            static_assert(N >= 1);

            if constexpr (N >= 3)
                return reactphysics3d::Vector3{ x, y, z };
            else if constexpr (N >= 2)
                return reactphysics3d::Vector3{ x, y, 0.f };
            else if constexpr (N >= 1)
                return greactphysics3d::Vector3{ x, 0.f, 0.f };
        }
        auto operator=(const glm::vec2& other) -> Vec<N>
        {
            static_assert(N >= 1);

            x = other.x;
            if constexpr (N >= 2)
                y = other.y;
            return *this;
        }
        auto operator=(const glm::vec3& other) -> Vec<N>
        {
            static_assert(N >= 1);

            x = other.x;
            if constexpr (N >= 2)
                y = other.y;
            else if constexpr (N >= 3)
                z = other.z;
            return *this;
        }
        auto operator=(const glm::vec4& other) -> Vec<N>
        {
            static_assert(N >= 1);

            x = other.x;
            if constexpr (N >= 2)
                y = other.y;
            else if constexpr (N >= 3)
                z = other.z;
            else if constexpr (N >= 4)
                w = other.w;
            return *this;
        }

        // 벡터 간 연산
        auto operator+(const Vec<N> other) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] + other.data[i];
            return result;
        }
        auto operator-(const Vec<N> other) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] - other.data[i];
            return result;
        }
        auto operator*(const Vec<N> other) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] * other.data[i];
            return result;
        }
        auto operator/(const Vec<N> other) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] / other.data[i];
            return result;
        }

        // 스칼라 연산
        auto operator+(float scalar) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] + scalar;
            return result;
        }
        auto operator-(float scalar) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] - scalar;
            return result;
        }
        auto operator*(float scalar) const-> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] * scalar;
            return result;
        }
        auto operator/(float scalar) const-> Vec<N>
        {
            assert(scalar != 0.f);
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] / scalar;
            return result;
        }
    };

    // 스칼라가 왼쪽인 연산
    template<std::size_t N>
    auto operator+(float scalar, const Vec<N> vec)-> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar + vec.data[i];
        return result;
    }

    template<std::size_t N>
    auto operator-(float scalar, const Vec<N> vec)-> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar - vec.data[i];
        return result;
    }

    template<std::size_t N>
    auto operator*(float scalar, const Vec<N> vec)-> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar * vec.data[i];
        return result;
    }

    template<std::size_t N>
    auto operator/(float scalar, const Vec<N> vec)-> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar / vec.data[i];
        return result;
    }

    using Vec2 = Vec<2>;
    using Vec3 = Vec<3>;
    using Vec4 = Vec<4>;
}//namespace