#pragma once

#include "Camera.h"

namespace sh::game
{
	class EditorCamera : public Camera
	{
		COMPONENT(EditorCamera)
	public:
		SH_GAME_API EditorCamera(GameObject& owner);

		SH_GAME_API void Start() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void SetFocus(bool bfocus);
		SH_GAME_API void FocusObject(const GameObject& obj);
		SH_GAME_API void FocusObject(const std::vector<const GameObject*>& objs);

		SH_GAME_API void SetDirection(float pitch, float yaw);
		SH_GAME_API void SetPosition(const game::Vec3& pos);
	private:
		void HandleMouseInput();
		void HandleLeftMouseDrag();
		void HandleMiddleMouseDrag();
		void Zoom();
		void ClampAngles();
		void UpdateCameraPosition();
		void FocusAABB(const render::AABB& aabb);
	private:
		PROPERTY(distance)
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
		bool bFocus = false;
	};
}//namespace