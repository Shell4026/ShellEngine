#include "Game/Component/FollowTarget.h"
#include "Game/Component/Transform.h"
#include "Game/World.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
namespace sh::game
{
	FollowTarget::FollowTarget(GameObject& owner) :
		Component(owner)
	{
	}
	SH_GAME_API void FollowTarget::Update()
	{
		if (!core::IsValid(target))
			return;

		glm::vec3 pos = gameObject.transform->GetWorldPosition();
		glm::vec3 targetPos = target->GetWorldPosition();

		const float disSqr = glm::distance2(pos, targetPos);
		if (disSqr < 1e-6)
			return;

		pos = glm::mix(pos, targetPos, speed * world.deltaTime);
		gameObject.transform->SetWorldPosition(pos);
		gameObject.transform->UpdateMatrix();
	}
}//namespace