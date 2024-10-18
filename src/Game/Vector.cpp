#include "PCH.h"
#include "Vector.h"

namespace sh::game
{
	SH_GAME_API Vec2::Vec2()
	{
		x = 0.f;
		y = 0.f;
	}
	SH_GAME_API Vec2::Vec2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	SH_GAME_API Vec2::Vec2(const Vec<2>& other)
	{
		x = other.data[0];
		y = other.data[1];
	}
	SH_GAME_API Vec2::Vec2(const glm::vec2& other)
	{
		x = other.x;
		y = other.y;
	}
	SH_GAME_API Vec2::Vec2(const reactphysics3d::Vector2& other)
	{
		x = other.x;
		y = other.y;
	}

	SH_GAME_API Vec2::operator glm::vec2() const
	{
		return glm::vec2{ x, y };
	}
	SH_GAME_API Vec2::operator reactphysics3d::Vector2() const
	{
		return reactphysics3d::Vector2{ x, y };
	}
	SH_GAME_API auto Vec2::operator=(const Vec<2>& other) -> Vec2&
	{
		x = other.data[0];
		y = other.data[1];
		return *this;
	}
	SH_GAME_API auto Vec2::operator=(const glm::vec2& other) -> Vec2&
	{
		x = other.x;
		y = other.y;
		return *this;
	}
	SH_GAME_API auto Vec2::operator=(const reactphysics3d::Vector2& other)->Vec2&
	{
		x = other.x;
		y = other.y;
		return *this;
	}

	SH_GAME_API Vec3::Vec3()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}
	SH_GAME_API Vec3::Vec3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	SH_GAME_API Vec3::Vec3(const Vec<3>& other)
	{
		x = other.data[0];
		y = other.data[1];
		z = other.data[2];
	}
	SH_GAME_API Vec3::Vec3(const glm::vec3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	SH_GAME_API Vec3::Vec3(const reactphysics3d::Vector3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}
	SH_GAME_API Vec3::operator glm::vec3() const
	{
		return glm::vec3{ x, y, z };
	}
	SH_GAME_API Vec3::operator reactphysics3d::Vector3() const
	{
		return reactphysics3d::Vector3{ x, y, z };
	}
	SH_GAME_API auto Vec3::operator=(const Vec<3>& other) -> Vec3&
	{
		x = other.data[0];
		y = other.data[1];
		z = other.data[2];
		return *this;
	}
	SH_GAME_API auto Vec3::operator=(const Vec3& other) -> Vec3&
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}
	SH_GAME_API auto Vec3::operator=(const glm::vec3& other) -> Vec3&
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}
	SH_GAME_API auto Vec3::operator=(const reactphysics3d::Vector3& other)->Vec3&
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	SH_GAME_API Vec4::Vec4()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;
	}
	SH_GAME_API Vec4::Vec4(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	SH_GAME_API Vec4::Vec4(const Vec<4>& other)
	{
		x = other.data[0];
		y = other.data[1];
		z = other.data[2];
		w = other.data[3];
	}
	SH_GAME_API Vec4::Vec4(const glm::vec4& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}
	SH_GAME_API Vec4::operator glm::vec4() const
	{
		return glm::vec4{ x, y, z, w };
	}
	SH_GAME_API auto Vec4::operator=(const Vec<4>& other) -> Vec4&
	{
		x = other.data[0];
		y = other.data[1];
		z = other.data[2];
		w = other.data[3];
		return *this;
	}
	SH_GAME_API auto Vec4::operator=(const Vec4& other)->Vec4&
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}
	SH_GAME_API auto Vec4::operator=(const glm::vec4& other) -> Vec4&
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}
}//namespace