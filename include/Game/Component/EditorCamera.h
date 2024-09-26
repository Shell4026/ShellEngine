﻿#pragma once

#include "Camera.h"

namespace sh::game
{
	class EditorCamera : public Camera
	{
		SCLASS(EditorCamera)
	private:
		float distance;

		float xdir;
		float lastXdir;
		float ydir;
		float lastYdir;

		float rotationSpeed;
		float moveSpeed;

		glm::vec3 lastLookPos;

		glm::vec2 leftPressedPos;
		glm::vec2 middlePressedPos;

		bool leftMousePressed;
		bool middleMousePressed;
	private:
		inline void HandleMouseInput();
		inline void HandleLeftMouseDrag();
		inline void HandleMiddleMouseDrag();
		inline void Zoom();
		inline void ClampAngles();
		inline void UpdateCameraPosition();
	public:
		SH_GAME_API EditorCamera();

		SH_GAME_API void Update() override;
	};
}//namespace