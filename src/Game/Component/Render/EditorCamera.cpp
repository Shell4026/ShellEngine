#include "Game/Component/Render/EditorCamera.h"
#include "Game/Input.h"
#include "Game/GameObject.h"
#include "Game/ImGUImpl.h"
#include "Game/Component/Render/MeshRenderer.h"

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

	SH_GAME_API void EditorCamera::Start()
	{
		const glm::vec3 forward = glm::normalize(glm::vec3{ gameObject.transform->GetWorldPosition() - lookPos });
		const float yaw = glm::degrees(atan2(forward.z, forward.x));
		const float pitch = glm::degrees(asin(forward.y));
		xdir = pitch;
		ydir = yaw;
		camera.SetFarPlane(10000.0f);
	}

	SH_GAME_API void EditorCamera::BeginUpdate()
	{
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
	SH_GAME_API void EditorCamera::FocusObject(const GameObject& obj)
	{
		bool bFirst = true;
		render::AABB combinedAABB;

		auto renderers = obj.GetComponentsInChildren<MeshRenderer>(true);
		for (const MeshRenderer* renderer : renderers)
		{
			if (renderer->GetMesh() == nullptr)
				continue;
			const render::AABB& worldAABB = renderer->GetWorldAABB();
			if (bFirst)
			{
				combinedAABB = worldAABB;
				bFirst = false;
			}
			else
				combinedAABB = render::AABB::Encapsulate(combinedAABB, worldAABB);
		}

		FocusAABB(combinedAABB);
	}
	SH_GAME_API void EditorCamera::FocusObject(const std::vector<const GameObject*>& objs)
	{
		bool bFirst = true;
		render::AABB combinedAABB;

		for (auto obj : objs)
		{
			if (!core::IsValid(obj))
				continue;
			auto renderers = obj->GetComponentsInChildren<MeshRenderer>(true);
			for (const MeshRenderer* renderer : renderers)
			{
				if (renderer->GetMesh() == nullptr)
					continue;
				const render::AABB& worldAABB = renderer->GetWorldAABB();
				if (bFirst)
				{
					combinedAABB = worldAABB;
					bFirst = false;
				}
				else
					combinedAABB = render::AABB::Encapsulate(combinedAABB, worldAABB);
			}
		}

		FocusAABB(combinedAABB);
	}
	SH_GAME_API void EditorCamera::SetDirection(float pitch, float yaw)
	{
		this->xdir = pitch;
		this->ydir = yaw;
	}
	SH_GAME_API void EditorCamera::SetPosition(const game::Vec3& pos)
	{
		gameObject.transform->SetWorldPosition(pos);
		const glm::vec3 forward = glm::normalize(glm::vec3{ gameObject.transform->GetWorldPosition() - lookPos });
		const float yaw = glm::degrees(atan2(forward.z, forward.x));
		const float pitch = glm::degrees(asin(forward.y));
		xdir = pitch;
		ydir = yaw;
		lastXdir = xdir;
		lastYdir = ydir;
	}

	void EditorCamera::HandleMouseInput()
	{
		const glm::vec2& mouseDelta = Input::mousePositionDelta;

		Zoom();

		if (Input::GetMousePressed(Input::MouseType::Left) ||
			Input::GetKeyPressed(Input::KeyCode::LAlt) && Input::GetMouseDown(Input::MouseType::Left))
		{
			leftPressedPos = Input::mousePosition;
			leftMousePressed = true;
		}
		if (Input::GetMouseReleased(Input::MouseType::Left) || Input::GetKeyReleased(Input::KeyCode::LAlt))
		{
			leftMousePressed = false;
			lastXdir = xdir;
			lastYdir = ydir;
		}

		if (Input::GetKeyDown(Input::KeyCode::LAlt))
		{
			
			if (Input::GetMouseDown(Input::MouseType::Left))
			{
				HandleLeftMouseDrag();
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
		if (!bFocus)
			return;
		glm::vec2 delta = Input::mousePosition - leftPressedPos;
		xdir = lastXdir + rotationSpeed * delta.y;
		ydir = lastYdir + rotationSpeed * delta.x;
	}

	void EditorCamera::HandleMiddleMouseDrag()
	{
		if (!bFocus)
			return;
		if (!middleMousePressed)
		{
			middleMousePressed = true;
			middlePressedPos = Input::mousePosition;
		}

		glm::vec3 to = lookPos - glm::vec3{ gameObject.transform->position };
		glm::vec3 right = glm::normalize(glm::cross(to, glm::vec3{ GetUpVector() }));
		glm::vec3 up = glm::normalize(glm::cross(right, to));

		glm::vec2 delta = (Input::mousePosition - middlePressedPos);
		float dis = glm::length(delta);

		lookPos = lastLookPos + right * (-delta.x * moveSpeed * distance * 0.1f) + up * (delta.y * moveSpeed * distance * 0.1f);
	}

	void EditorCamera::Zoom()
	{
		if (!bFocus)
			return;
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
		xdir = (xdir >= 360.f) ?
			xdir - 360.f : (xdir <= 0.f) ?
			xdir + 360.f : xdir;

		ydir = (ydir >= 360.f) ?
			ydir - 360.f : (ydir <= 0.f) ?
			ydir + 360.f : ydir;

		glm::vec3 v = gameObject.transform->GetWorldPosition() - lookPos;
		//glm::vec3 up = glm::cross(glm::vec3{ 1.0f, 0.f, 0.f }, glm::normalize(v));
		Vec3 up = camera.GetUpVector(core::ThreadType::Game);
		up.y = (xdir >= 90 && xdir <= 270) ? -1 : 1;
		camera.SetUpVector(up);
	}

	void EditorCamera::UpdateCameraPosition()
	{
		float x = lookPos.x + distance * glm::cos(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		float y = lookPos.y + distance * glm::sin(glm::radians(xdir));
		float z = lookPos.z + distance * glm::sin(glm::radians(ydir)) * glm::cos(glm::radians(xdir));
		gameObject.transform->SetPosition(x, y, z);
	}

	void EditorCamera::FocusAABB(const render::AABB& aabb)
	{
		// 원근 투영기준, 카메라의 절두체에 바운딩 박스를 구체로 바꾸고 딱 들어 맞는 거리를 찾는 코드
		// 원리: 카메라와 구체 중심의 거리 d, 구체 반지름 r
		// sin(theta) = r/d
		// d = r / sin(theta)
		const float padding = 1.2f;
		const float r = aabb.GetRadius();

		const float fovy = glm::radians(GetFov());
		const float fovx = glm::radians(GetFovx());

		const float thetaV = fovy / 2.f;
		const float thetah = fovx / 2.f;
		const float theta = std::min(fovy * 0.5f, fovx * 0.5f);

		const float d = (r / sin(theta)) * padding;

		SetLookPos(aabb.GetCenter());
		distance = d;
	}
}//namespace