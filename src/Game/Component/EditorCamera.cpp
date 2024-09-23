#include "Game/Component/EditorCamera.h"

#include "Game/Input.h"
#include "Game/GameObject.h"
namespace sh::game
{
	EditorCamera::EditorCamera() :
		distance(3.f),
		xdir(45.f), ydir(90.f),
		lastXdir(xdir), lastYdir(ydir),
		leftPressedPos(), middlePressedPos(),
		lastLookPos(lookPos),
		rotationSpeed(0.2f), moveSpeed(0.02f)
	{
		auto& type = GetStaticType();
	}

	void EditorCamera::HandleMouseInput()
	{
		const glm::vec2& mouseDelta = Input::mousePositionDelta;

		Zoom();

		if (Input::GetKeyDown(Input::KeyCode::LAlt))
		{
			if (Input::GetMouseDown(Input::MouseType::Left))
			{
				HandleLeftMouseDrag();
			}
			else
			{
				leftMousePressed = false;
				lastXdir = xdir;
				lastYdir = ydir;
			}

			if (Input::GetMouseDown(Input::MouseType::Middle))
			{
				HandleMiddleMouseDrag();
			}
			else
			{
				middleMousePressed = false;
				lastLookPos = lookPos;
			}
		}
	}

	void EditorCamera::HandleLeftMouseDrag()
	{
		if (!leftMousePressed)
		{
			leftMousePressed = true;
			leftPressedPos = Input::mousePosition;
		}

		glm::vec2 delta = Input::mousePosition - leftPressedPos;
		xdir = lastXdir + rotationSpeed * delta.y;
		ydir = lastYdir + rotationSpeed * delta.x;
	}

	void EditorCamera::HandleMiddleMouseDrag()
	{
		if (!middleMousePressed)
		{
			middleMousePressed = true;
			middlePressedPos = Input::mousePosition;
		}

		glm::vec3 to = lookPos - gameObject->transform->position;
		glm::vec3 right = glm::normalize(glm::cross(to, this->up));
		glm::vec3 up = glm::normalize(glm::cross(right, to));

		glm::vec2 delta = (Input::mousePosition - middlePressedPos);
		float dis = glm::length(delta);
		
		lookPos = lastLookPos + right * (-delta.x * moveSpeed) + up * (delta.y * moveSpeed);
	}

	void EditorCamera::Zoom()
	{
		distance -= Input::mouseWheelDelta;
	}

	void EditorCamera::ClampAngles()
	{
		xdir = (xdir >= 360.f) ? xdir - 360.f : (xdir < 0) ? xdir + 360.f : xdir;
		ydir = (ydir >= 360.f) ? ydir - 360.f : (ydir < 0) ? ydir + 360.f : ydir;

		up.y = (xdir >= 90 && xdir < 270) ? -1 : 1;
	}

	void EditorCamera::UpdateCameraPosition()
	{
		float x = lookPos.x + distance * glm::cos(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		float y = lookPos.y + distance * glm::sin(glm::radians(xdir));
		float z = lookPos.z + distance * glm::sin(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		gameObject->transform->SetPosition(x, y, z);
	}

	void EditorCamera::Update()
	{
		HandleMouseInput();
		ClampAngles();
		UpdateCameraPosition();
		Super::Update();
	}
}//namespace