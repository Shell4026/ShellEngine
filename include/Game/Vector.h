#pragma once
#include "Core/ISerializable.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <cassert>
#include <type_traits>
#include <initializer_list>
#include <tuple>
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

    template<std::size_t N>
    struct Vec
    {
    private:
        struct Empty {};
    public:
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
        
        Vec()
        {
            for (std::size_t i = 0; i < N; ++i)
                data[i] = 0.f;
        }
        Vec(const std::initializer_list<float>& list)
        {
            for (std::size_t i = 0; i < N; ++i)
            {
                if (i < list.size())
                    data[i] = *(list.begin() + i);
            }
        }
        Vec(const glm::vec2& vec)
        {
            assert(N >= 2);
            if constexpr (N >= 2)
            {
                x = vec.x;
                y = vec.y;
            }
        }
        Vec(const glm::vec3& vec)
        {
            if constexpr (N >= 2)
            {
                x = vec.x;
                y = vec.y;
            }
            if constexpr (N >= 3)
                z = vec.z;
        }
        Vec(const glm::vec4& vec)
        {
            if constexpr (N >= 2)
            {
                x = vec.x;
                y = vec.y;
            }
            if constexpr (N >= 3)
                z = vec.z;
            if constexpr (N >= 4)
                w = vec.w;
        }

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
        bool operator==(const Vec<N>& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
        bool operator!=(const Vec<N>& other) const
        {
            return !operator==(other);
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
            if constexpr (N >= 3)
                z = other.z;
            return *this;
        }
        auto operator=(const glm::vec4& other) -> Vec<N>
        {
            static_assert(N >= 1);

            x = other.x;
            if constexpr (N >= 2)
                y = other.y;
            if constexpr (N >= 3)
                z = other.z;
            if constexpr (N >= 4)
                w = other.w;
            return *this;
        }
        auto operator=(const std::initializer_list<float>& list) -> Vec<N>
        {
            for (std::size_t i = 0; i < N; ++i)
            {
                if (i < list.size())
                    data[i] = *(list.begin() + i);
            }
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

    template <std::size_t I>
    constexpr auto get(Vec2& v) noexcept -> float&
    {
        if constexpr (I == 0) return v.x;
        else return v.y;
    }
    template <std::size_t I>
    constexpr auto get(const Vec2& v) noexcept -> const float&
    {
        if constexpr (I == 0) return v.x;
        else return v.y;
    }
    template <std::size_t I>
    constexpr auto get(Vec2&& v) noexcept -> float&&
    {
        if constexpr (I == 0) return std::move(v.x);
        else return std::move(v.y);
    }

    template <std::size_t I>
    constexpr auto get(Vec3& v) noexcept -> float&
    {
        if constexpr (I == 0) return v.x;
        else if constexpr (I == 1) return v.y;
        else return v.z;
    }
    template <std::size_t I>
    constexpr auto get(const Vec3& v) noexcept -> const float&
    {
        if constexpr (I == 0) return v.x;
        else if constexpr (I == 1) return v.y;
        else return v.z;
    }
    template <std::size_t I>
    constexpr auto get(Vec3&& v) noexcept -> float&&
    {
        if constexpr (I == 0) return std::move(v.x);
        else if constexpr (I == 1) return std::move(v.y);
        else return std::move(v.z);
    }

    template <std::size_t I>
    constexpr auto get(Vec4& v) noexcept -> float&
    {
        if constexpr (I == 0) return v.x;
        else if constexpr (I == 1) return v.y;
        else if constexpr (I == 2) return v.z;
        else return v.w;
    }
    template <std::size_t I>
    constexpr auto get(const Vec4& v) noexcept -> const float&
    {
        if constexpr (I == 0) return v.x;
        else if constexpr (I == 1) return v.y;
        else if constexpr (I == 2) return v.z;
        else return v.w;
    }
    template <std::size_t I>
    constexpr auto get(Vec4&& v) noexcept -> float&&
    {
        if constexpr (I == 0) return std::move(v.x);
        else if constexpr (I == 1) return std::move(v.y);
        else if constexpr (I == 2) return std::move(v.z);
        else return std::move(v.w);
    }
}//namespace

namespace sh::core
{
    // 직렬화, 역직렬화

    template<std::size_t n>
    inline void SerializeProperty(core::Json& json, const std::string& key, const game::Vec<n>& vec)
    {
        for (int i = 0; i < n; ++i)
            json[key].push_back(vec.data[i]);
    }

    template<std::size_t n>
    inline auto DeserializeProperty(const core::Json& json, const std::string& key, game::Vec<n>& vec) -> bool
    {
        if (json.contains(key) && json[key].is_array() && json[key].size() == n) 
        {
            for(std::size_t i = 0; i < n; ++i)
                vec.data[i] = json[key][i].get<float>();
            return true;
        }
        return false;
    }
}//namespace

namespace std // 구조적 바인딩 지원
{
    template<>
    struct tuple_size<sh::game::Vec2> : std::integral_constant<std::size_t, 2> {};
    template<>
    struct tuple_size<sh::game::Vec3> : std::integral_constant<std::size_t, 3> {};
    template<>
    struct tuple_size<sh::game::Vec4> : std::integral_constant<std::size_t, 4> {};

    template<std::size_t I>
    struct tuple_element<I, sh::game::Vec2> { using type = float; };
    template<std::size_t I>
    struct tuple_element<I, const sh::game::Vec2> { using type = float; };

    template<std::size_t I>
    struct tuple_element<I, sh::game::Vec3> { using type = float; };
    template<std::size_t I>
    struct tuple_element<I, const sh::game::Vec3> { using type = float; };

    template<std::size_t I>
    struct tuple_element<I, sh::game::Vec4> { using type = float; };
    template<std::size_t I>
    struct tuple_element<I, const sh::game::Vec4> { using type = float; };
}//namespace std