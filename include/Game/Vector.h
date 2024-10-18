#pragma once

#include "Export.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "reactphysics3d/mathematics/Vector2.h"
#include "reactphysics3d/mathematics/Vector3.h"

#include <cassert>

namespace sh::game
{
	template<std::size_t N>
	struct Vec
	{
		float data[N];

        auto operator=(const Vec<N>& other) -> Vec<N>&
        {
            for (std::size_t i = 0; i < N; ++i)
                data[i] = other.data[i];
            return *this;
        }

        // 벡터 간 연산
        auto operator+(const Vec<N>& other) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] + other.data[i];
            return result;
        }
        auto operator-(const Vec<N>& other) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] - other.data[i];
            return result;
        }
        auto operator*(const Vec<N>& other) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] * other.data[i];
            return result;
        }
        auto operator/(const Vec<N>& other) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] / other.data[i];
            return result;
        }
        // 스칼라 연산
        auto operator+(float scalar) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] + scalar;
            return result;
        }
        auto operator-(float scalar) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] - scalar;
            return result;
        }
        auto operator*(float scalar) const -> Vec<N>
        {
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] * scalar;
            return result;
        }
        auto operator/(float scalar) const -> Vec<N>
        {
            assert(scalar != 0.f);
            Vec<N> result{};
            for (std::size_t i = 0; i < N; ++i)
                result.data[i] = this->data[i] / scalar;
            return result;
        }
	};
    // 스칼라가 왼쪽
    template<std::size_t N>
    static auto operator+(float scalar, const Vec<N>& vec) -> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar + vec.data[i];
        return result;
    }
    template<std::size_t N>
    static auto operator-(float scalar, const Vec<N>& vec) -> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar - vec.data[i];
        return result;
    }
    template<std::size_t N>
    static auto operator*(float scalar, const Vec<N>& vec) -> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar * vec.data[i];
        return result;
    }
    template<std::size_t N>
    static auto operator/(float scalar, const Vec<N>& vec) -> Vec<N>
    {
        Vec<N> result{};
        for (std::size_t i = 0; i < N; ++i)
            result.data[i] = scalar / vec.data[i];
        return result;
    }

    struct Vec2 : Vec<2>
    {
        float& x = data[0];
        float& y = data[1];

        SH_GAME_API Vec2();
        SH_GAME_API Vec2(float x, float y);
        SH_GAME_API Vec2(const Vec<2>& other);
        SH_GAME_API Vec2(const glm::vec2& other);
        SH_GAME_API Vec2(const reactphysics3d::Vector2& other);

        SH_GAME_API operator glm::vec2() const;
        SH_GAME_API operator reactphysics3d::Vector2() const;

        SH_GAME_API auto operator=(const Vec<2>& other) -> Vec2&;
        SH_GAME_API auto operator=(const Vec2& other) -> Vec2&;
        SH_GAME_API auto operator=(const glm::vec2& other) -> Vec2&;
        SH_GAME_API auto operator=(const reactphysics3d::Vector2& other) -> Vec2&;
    };
    struct Vec3 : Vec<3>
    {
        float& x = data[0];
        float& y = data[1];
        float& z = data[2];

        SH_GAME_API Vec3();
        SH_GAME_API Vec3(float x, float y, float z);
        SH_GAME_API Vec3(const Vec<3>& other);
        SH_GAME_API Vec3(const glm::vec3& other);
        SH_GAME_API Vec3(const reactphysics3d::Vector3& other);

        SH_GAME_API Vec3::operator glm::vec3() const;
        SH_GAME_API Vec3::operator reactphysics3d::Vector3() const;

        SH_GAME_API auto operator=(const Vec<3>& other) -> Vec3&;
        SH_GAME_API auto operator=(const Vec3& other) -> Vec3&;
        SH_GAME_API auto operator=(const glm::vec3& other) -> Vec3&;
        SH_GAME_API auto operator=(const reactphysics3d::Vector3& other) -> Vec3&;
    };
    struct Vec4 : Vec<4>
    {
        float& x = data[0];
        float& y = data[1];
        float& z = data[2];
        float& w = data[3];

        SH_GAME_API Vec4();
        SH_GAME_API Vec4(float x, float y, float z, float w);
        SH_GAME_API Vec4(const Vec<4>& other);
        SH_GAME_API Vec4(const glm::vec4& other);

        SH_GAME_API operator glm::vec4() const;

        SH_GAME_API auto operator=(const Vec<4>& other) -> Vec4&;
        SH_GAME_API auto operator=(const Vec4& other) -> Vec4&;
        SH_GAME_API auto operator=(const glm::vec4& other) -> Vec4&;
    };
}//namespace