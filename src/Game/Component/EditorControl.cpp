#include "PCH.h"
#include "Component/EditorControl.h"
#include "Component/Camera.h"
#include "Component/LineRenderer.h"

#include "GameObject.h"
#include "World.h"
#include "Input.h"
#include "SMath.h"

#include "Physics/Ray.h"

namespace sh::game
{
	SH_GAME_API EditorControl::EditorControl(GameObject& owner) :
		Component(owner)
	{
		if (auto obj = world.GetGameObject("_Helper"); obj == nullptr)
		{
			obj = world.AddGameObject("_Helper");
			obj->hideInspector = true;
			helper = obj->AddComponent<LineRenderer>();
			helper->SetColor(Vec4{ 1.f, 0.f, 0.f, 1.f });
			helper->SetStart(Vec3{ -100.f, 0.f, 0.f });
			helper->SetEnd(Vec3{ 100.f, 0.f, 0.f });

			obj->SetActive(false);
		}
		else
			helper = obj->GetComponent<LineRenderer>();
	}

	inline void EditorControl::MoveControl()
	{
		glm::vec3 linePoint = camera->gameObject.transform->position;
		glm::vec3 facePoint = gameObject.transform->position;

		auto clickRay = camera->ScreenPointToRay(clickPos);
		glm::vec3 clickHit = SMath::GetPlaneCollisionPoint(clickRay.origin, clickRay.direction, facePoint, forward).value_or(glm::vec3{ 0.f });
		auto ray = camera->ScreenPointToRay(Input::mousePosition);
		glm::vec3 hit = SMath::GetPlaneCollisionPoint(ray.origin, ray.direction, facePoint, forward).value_or(glm::vec3{ 0.f });

		glm::vec3 delta = hit - clickHit;

		float deltaX = 0.f;
		float deltaY = 0.f;
		glm::vec3 pos{ 0.f };

		if (axis == Axis::None)
		{
			deltaX = glm::dot(delta, glm::vec3{ right });
			deltaY = glm::dot(delta, glm::vec3{ up });
			pos = posLast + right * deltaX + up * deltaY;
		}
		else if (axis == Axis::X)
		{
			pos = posLast;
			float deltaX = glm::dot(glm::vec3{ 1.f, 0.f, 0.f }, delta);
			pos.x = posLast.x + deltaX;
		}
		else if (axis == Axis::Y)
		{
			pos = posLast;
			float deltaY = glm::dot(glm::vec3{ 0.f, 1.f, 0.f }, delta);
			pos.y = posLast.y + deltaY;
		}
		else if (axis == Axis::Z)
		{
			pos = posLast;
			float deltaZ = glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, delta);
			pos.z = posLast.z + deltaZ;
		}

		gameObject.transform->SetPosition(pos);
	}
	inline void EditorControl::ScaleControl()
	{
		glm::vec3 linePoint = camera->gameObject.transform->position;
		glm::vec3 facePoint = gameObject.transform->position;

		auto clickRay = camera->ScreenPointToRay(clickPos);
		glm::vec3 clickHit = SMath::GetPlaneCollisionPoint(clickRay.origin, clickRay.direction, facePoint, forward).value_or(glm::vec3{ 0.f });
		auto ray = camera->ScreenPointToRay(Input::mousePosition);
		glm::vec3 hit = SMath::GetPlaneCollisionPoint(ray.origin, ray.direction, facePoint, forward).value_or(glm::vec3{ 0.f });

		glm::vec3 facePointToHit = hit - facePoint;
		glm::vec3 delta = hit - clickHit;
		float len = glm::length(delta);
		if (glm::dot(facePointToHit, delta) < 0)
			len = -len;

		glm::vec3 scale = scaleLast;
		if (axis == Axis::None)
			scale += len;
		else if (axis == Axis::X)
			scale.x += len;
		else if (axis == Axis::Y)
			scale.y += len;
		else if (axis == Axis::Z)
			scale.z += len;

		gameObject.transform->SetScale(scale);
	}
	inline void EditorControl::RotateControl()
	{
		glm::vec2 screenPos = glm::project(glm::vec3{ gameObject.transform->GetWorldPosition() }, 
			camera->GetViewMatrix(), camera->GetProjMatrix(),
			glm::vec4{ gameObject.world.renderer.GetViewportStart().x, gameObject.world.renderer.GetViewportStart().y, gameObject.world.renderer.GetViewportEnd().x, gameObject.world.renderer.GetViewportEnd().y });

		screenPos.y = gameObject.world.renderer.GetViewportEnd().y - screenPos.y;
		
		glm::vec2 v = glm::normalize(screenPos - glm::vec2{ clickPos });
		glm::vec2 v2 = glm::normalize(screenPos - Input::mousePosition);

		float angleRad = glm::acos(glm::dot(v, v2));

		float crossProduct = v.x * v2.y - v.y * v2.x;
		if (crossProduct < 0)
			angleRad = -angleRad;

		gameObject.transform->SetRotation(glm::angleAxis(angleRad, glm::vec3{ forward }) * quatLast);
	}

	SH_GAME_API void EditorControl::Update()
	{
		if (!core::IsValid(camera))
			return;

		if (mode != Mode::Move)
		{
			if (Input::GetKeyDown(Input::KeyCode::G))
			{
				forward = glm::normalize(glm::vec3{ camera->GetLookPos() - camera->gameObject.transform->position });
				up = camera->GetUpVector();
				right = glm::normalize(glm::cross(glm::vec3{ forward }, glm::vec3{ up }));
				up = glm::normalize(glm::cross(glm::vec3{ right }, glm::vec3{ forward }));

				posLast = gameObject.transform->position;
				clickPos = Input::mousePosition;
				mode = Mode::Move;
			}
		}
		if (mode != Mode::Scale)
		{
			if (Input::GetKeyDown(Input::KeyCode::S))
			{
				forward = glm::normalize(glm::vec3{ camera->GetLookPos() - camera->gameObject.transform->position });
				up = camera->GetUpVector();
				right = glm::normalize(glm::cross(glm::vec3{ forward }, glm::vec3{ up }));
				up = glm::normalize(glm::cross(glm::vec3{ right }, glm::vec3{ forward }));

				scaleLast = gameObject.transform->scale;
				clickPos = Input::mousePosition;
				mode = Mode::Scale;
			}
		}
		if (mode != Mode::Rotate)
		{
			if (Input::GetKeyDown(Input::KeyCode::R))
			{
				forward = glm::normalize(glm::vec3{ camera->GetLookPos() - camera->gameObject.transform->position });
				up = camera->GetUpVector();
				right = glm::normalize(glm::cross(glm::vec3{ forward }, glm::vec3{ up }));
				up = glm::normalize(glm::cross(glm::vec3{ right }, glm::vec3{ forward }));

				quatLast = gameObject.transform->GetQuat();
				clickPos = Input::mousePosition;
				mode = Mode::Rotate;
			}
		}
		if (mode != Mode::None)
		{
			if (Input::GetKeyDown(Input::KeyCode::X))
			{
				axis = Axis::X;
				helper->gameObject.SetActive(true);
				helper->SetColor(Vec4{ 1.f, 0.f, 0.f, 1.f });
				helper->SetStart(Vec3{ -100.f, 0.f, 0.f } + posLast);
				helper->SetEnd(Vec3{ 100.f, 0.f, 0.f } + posLast);
			}
			else if (Input::GetKeyDown(Input::KeyCode::Y))
			{
				axis = Axis::Y;
				helper->gameObject.SetActive(true);
				helper->SetColor(Vec4{ 0.f, 1.f, 0.f, 1.f });
				helper->SetStart(Vec3{ 0.f, -100.f, 0.f } + posLast);
				helper->SetEnd(Vec3{ 0.f, 100.f, 0.f } + posLast);
			}
			else if (Input::GetKeyDown(Input::KeyCode::Z))
			{
				axis = Axis::Z;
				helper->gameObject.SetActive(true);
				helper->SetColor(Vec4{ 0.f, 0.f, 1.f, 1.f });
				helper->SetStart(Vec3{ 0.f, 0.f, -100.f } + posLast);
				helper->SetEnd(Vec3{ 0.f, 0.f, 100.f } + posLast);
			}

			if (Input::GetMouseDown(Input::MouseType::Right))
			{
				if (mode == Mode::Move)
					gameObject.transform->SetPosition(posLast);
				if (mode == Mode::Scale)
					gameObject.transform->SetScale(scaleLast);
				if (mode == Mode::Rotate)
					gameObject.transform->SetRotation(quatLast);
				mode = Mode::None;
				axis = Axis::None;
				if (helper)
				{
					helper->gameObject.SetActive(false);
				}
			}
			else if (Input::GetMouseDown(Input::MouseType::Left))
			{
				mode = Mode::None;
				axis = Axis::None;
				if (helper)
				{
					helper->gameObject.SetActive(false);
				}
			}
		}

		switch (mode)
		{
		case Mode::Move:
			MoveControl();
			break;
		case Mode::Scale:
			ScaleControl();
			break;
		case Mode::Rotate:
			RotateControl();
			break;
		}
	}

	SH_GAME_API void EditorControl::SetCamera(Camera* camera)
	{
		this->camera = camera;
	}
}//namespace