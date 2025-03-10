#include "PCH.h"
#include "Game/Component/EditorCamera.h"

#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/ImGUImpl.h"

namespace sh::game
{
	SH_GAME_API EditorCamera::EditorCamera(GameObject& owner) :
		Camera(owner),

		distance(5.f),
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

		glm::vec3 to = lookPos - glm::vec3{ gameObject.transform->position };
		glm::vec3 right = glm::normalize(glm::cross(to, glm::vec3{GetUpVector()}));
		glm::vec3 up = glm::normalize(glm::cross(right, to));

		glm::vec2 delta = (Input::mousePosition - middlePressedPos);
		float dis = glm::length(delta);
		
		lookPos = lastLookPos + right * (-delta.x * moveSpeed * distance * 0.1f) + up * (delta.y * moveSpeed * distance * 0.1f);
	}

	void EditorCamera::Zoom()
	{
		float delta = 1.f;
		if (Input::mouseWheelDelta > 0)
			delta = 0.9f;
		else if (Input::mouseWheelDelta < 0)
			delta = 1.1f;

		distance = delta * distance;
		if (distance < 0.01f) distance = 0.01f;
	}

	void EditorCamera::ClampAngles()
	{
		xdir = (xdir >= 360.f) ? xdir - 360.f : (xdir < 0) ? xdir + 360.f : xdir;
		ydir = (ydir >= 360.f) ? ydir - 360.f : (ydir < 0) ? ydir + 360.f : ydir;

		Vec3 up = camera.GetUpVector();
		up.y = (xdir >= 90 && xdir < 270) ? -1 : 1;
		camera.SetUpVector(up);
	}

	void EditorCamera::UpdateCameraPosition()
	{
		float x = lookPos.x + distance * glm::cos(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		float y = lookPos.y + distance * glm::sin(glm::radians(xdir));
		float z = lookPos.z + distance * glm::sin(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		gameObject.transform->SetPosition(x, y, z);
	}

	SH_GAME_API void EditorCamera::Start()
	{
		glm::vec3 v = gameObject.transform->GetWorldPosition() - lookPos;
		glm::vec3 cross = glm::cross(glm::vec3{ 1.0f, 0.f, 0.f }, glm::normalize(glm::vec3{ v.x, 0.f, v.z }));
		xdir = glm::degrees(glm::asin(glm::length(cross)));
		cross = glm::cross(glm::vec3{ 1.0f, 0.f, 0.f }, glm::normalize(glm::vec3{ v.x, v.y, 0.f }));
		ydir = glm::degrees(glm::asin(glm::length(cross)));
	}

	SH_GAME_API void EditorCamera::BeginUpdate()
	{
		if (bFocus)
			HandleMouseInput();
		ClampAngles();
		UpdateCameraPosition();
		Super::BeginUpdate();
	}

	SH_GAME_API void EditorCamera::Update()
	{
	}

	SH_GAME_API void EditorCamera::SetFocus(bool bfocus)
	{
		this->bFocus = bfocus;
	}
}//namespace