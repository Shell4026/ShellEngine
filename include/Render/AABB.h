#pragma once

#include "Export.h"

#include "glm/vec3.hpp"

namespace sh::render
{
	/// @brief 단순한 AABB 클래스
	class AABB
	{
	private:
		glm::vec3 min, max, center;
	public:
		SH_RENDER_API AABB();
		SH_RENDER_API AABB(float x1, float y1, float z1, float x2, float y2, float z2);
		SH_RENDER_API AABB(const glm::vec3& min, const glm::vec3& max);

		SH_RENDER_API void Set(const glm::vec3& min, const glm::vec3& max);
		/// @brief min과 max를 amount만큼 늘리는 함수.
		/// @param amount 얼마만큼 늘릴지
		SH_RENDER_API void Expand(float amount);

		/// @brief 점이 AABB내에 존재하는지 확인하는 함수.
		/// @param point 3차원 점
		/// @return 존재 시 true, 아닐 시 false
		SH_RENDER_API bool Contains(const glm::vec3& point) const;
		/// @brief 다른 바운딩 박스와 충돌하는지 확인하는 함수.
		/// @param other 다른 바운딩 박스
		/// @return 충돌 시 true, 아닐 시 false
		SH_RENDER_API bool Intersects(const AABB& other) const;

		SH_RENDER_API auto GetMin() const -> const glm::vec3&;
		SH_RENDER_API auto GetMax() const -> const glm::vec3&;
		SH_RENDER_API auto GetCenter() const -> const glm::vec3&;
		SH_RENDER_API auto GetWorldAABB(const glm::mat4& modelMatrix) const -> AABB;
	};
}//namespace