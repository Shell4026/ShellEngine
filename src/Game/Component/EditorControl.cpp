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
	EditorControl::EditorControl(GameObject& owner) :
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

	void EditorControl::Update()
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
		else
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
		}
		if (mode != Mode::None)
		{
			if (Input::GetMouseDown(Input::MouseType::Right))
			{
				gameObject.transform->SetPosition(posLast);
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
		}
	}

	SH_GAME_API void EditorControl::SetCamera(Camera* camera)
	{
		this->camera = camera;
	}
}//namespace