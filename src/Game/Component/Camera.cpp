#include "Component/Camera.h"
#include "GameObject.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cstdint>

namespace sh::game
{
	Camera::Camera() :
		worldToCameraMatrix(matView),
		matProj(), matView(),
		fov(45.f), nearPlane(0.1f), farPlane(1000.f),
		cameraHandle(0), depth(-1),

		renderTexture(nullptr)
	{
	}
	Camera::~Camera()
	{
		if (gameObject->world.GetMainCamera() == this)
		{
			//다음에 추가된 카메라를 메인 카메라로 한다.
			for (auto& cam : gameObject->world.GetCameras())
			{
				if (cam == this)
					continue;

				gameObject->world.SetMainCamera(cam);
				break;
			}
		}
		gameObject->world.UnRegisterCamera(this);
		gameObject->world.renderer.DeleteCamera(cameraHandle);
	}
	
	void Camera::Awake()
	{
		Super::Awake();
		cameraHandle = gameObject->world.renderer.AddCamera(depth);
		gameObject->world.RegisterCamera(this);
	}
	void Camera::Start()
	{
	}
	void Camera::Update()
	{
		glm::vec2 size{};
		if (renderTexture == nullptr)
			size = gameObject->world.renderer.GetViewportEnd() - gameObject->world.renderer.GetViewportStart();
		else
			size = renderTexture->GetSize();
		
		matProj = glm::perspectiveFov(fov,
			static_cast<float>(size.x),
			static_cast<float>(size.y),
			nearPlane, farPlane);
		matView = glm::lookAt(gameObject->transform->position, glm::vec3(0.f), glm::vec3(0.f, 1.0f, 0.f));
	}

	auto Camera::GetProjMatrix() const -> const glm::mat4&
	{
		return matProj;
	}

	auto Camera::GetViewMatrix() const -> const glm::mat4&
	{
		return matView;
	}

	auto Camera::GetCameraHandle() const -> uint32_t
	{
		return cameraHandle;
	}

	void Camera::SetDepth(int depth)
	{
		this->depth = depth;
		gameObject->world.renderer.SetCameraDepth(cameraHandle, depth);
	}

#ifdef SH_EDITOR
	void Camera::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (strcmp(prop.GetName(), "depth") == 0)
		{
			gameObject->world.renderer.SetCameraDepth(cameraHandle, depth);
		}
	}
#endif
}