#include "PCH.h"
#include "SMath.h"

namespace sh::game
{
	SH_GAME_API auto SMath::GetPlaneCollisionPoint(const glm::vec3& linePoint, const glm::vec3& lineDir, const glm::vec3& planePoint, const glm::vec3& planeNormal) -> std::optional<glm::vec3>
	{
        float cos = glm::dot(planeNormal, lineDir);

        if (glm::abs(cos) < 1e-6f)
        {
            if (glm::dot(planeNormal, linePoint - planePoint) == 0.0f) 
            {
                return {}; // 선이 평면 위에 있음
            }
            else {
                return {}; // 교차점 없음
            }
        }

        float t = glm::dot(planeNormal, planePoint - linePoint) / cos;

        // 교차점 계산
        glm::vec3 intersection = linePoint + t * lineDir;

        return intersection;
	}
}
