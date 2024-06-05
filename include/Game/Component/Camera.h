#pragma once

#include "Component.h"
#include "Game/Export.h"

#include "glm/mat4x4.hpp"

namespace sh::game
{
	class Camera : public Component
	{
		SCOMPONENT(Camera)
	private:
		glm::mat4 matProj;
		glm::mat4 matView;
	public:
		float fov;
		float nearPlane;
		float farPlane;

		const glm::mat4& worldToCameraMatrix;
	public:
		SH_GAME_API Camera();
		SH_GAME_API ~Camera() = default;
		
		SH_GAME_API void Start() override;
		
		SH_GAME_API auto GetProjMatrix() const -> const glm::mat4&;
		SH_GAME_API auto GetViewMatrix() const -> const glm::mat4&;
	};
}