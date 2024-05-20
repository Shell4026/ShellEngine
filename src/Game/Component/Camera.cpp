#include "Component/Camera.h"
#include "GameObject.h"

#include "glm/gtc/matrix_transform.hpp"

namespace sh::game
{
	Camera::Camera(GameObject& owner) :
		Component(owner),
		worldToCameraMatrix(matView),
		matProj(), matView(),
		fov(45.f), nearPlane(0.1f), farPlane(1000.f)
	{
	}

	void Camera::Start()
	{
		matProj = glm::perspectiveFov(fov, 
			static_cast<float>(gameObject.world.renderer.GetWidth()), 
			static_cast<float>(gameObject.world.renderer.GetHeight()), 
			nearPlane, farPlane);
		matView = glm::lookAt(gameObject.transform->position, glm::vec3(0.f), glm::vec3(0.f, 1.0f, 0.f));
	}

	auto Camera::GetProjMatrix() const -> const glm::mat4&
	{
		return matProj;
	}

	auto Camera::GetViewMatrix() const -> const glm::mat4&
	{
		return matView;
	}
}