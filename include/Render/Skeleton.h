#pragma once
#include "Export.h"

#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace sh::render
{
	/// @brief SkinnedMeshRenderer가 런타임에 사용하는 스킨 바인드 포즈 데이터.
	/// 조인트 트랜스폼은 실제 씬의 bone GameObject(Transform)에서 읽는다.
	struct Skeleton
	{
		struct Joint
		{
			int nodeIdx = -1;
			glm::mat4 inverseBindMat{ 1.f };
		};
		std::vector<Joint> joints;

		auto GetJointCount() const -> std::size_t { return joints.size(); }
	};
}//namespace
