#include "UICamera.h"

#include "glm/gtc/matrix_transform.hpp"
namespace sh::game
{
	UICamera::UICamera()
	{
		SetOrthographic(true);
		pos[core::ThreadType::Render] = pos[core::ThreadType::Game] = { 0.f, 0.f, 0.0f };
		to[core::ThreadType::Render] = to[core::ThreadType::Game] = { 0.f, 0.f, -1.0f };

		bufferData[core::ThreadType::Game].matProj =
			glm::orthoRH_ZO(0.f, 13.66f, 0.0f, 7.68f, 0.01f, 1000.f);
		bufferData[core::ThreadType::Game].matView = glm::mat4{ 1.f };
		bufferData[core::ThreadType::Render] = bufferData[core::ThreadType::Game];
	}
	SH_GAME_API void UICamera::UpdateViewMatrix()
	{
	}
	SH_GAME_API void UICamera::UpdateProjMatrix()
	{
		bufferData[core::ThreadType::Game].matProj =
			glm::orthoRH_ZO(0.f, width[core::ThreadType::Game], 0.0f, height[core::ThreadType::Game], nearPlane[core::ThreadType::Game], farPlane[core::ThreadType::Game]);
		dirtyMask |= 256;
		SyncDirty();
	}
}//namepsace