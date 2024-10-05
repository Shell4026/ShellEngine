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
		camera(), depth(0), lookPos(), up(0, 1, 0),

		renderTexture(nullptr)
	{
	}
	Camera::~Camera()
	{
	}
	
	void Camera::Awake()
	{
		Super::Awake();
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
		matView = glm::lookAt(gameObject->transform->position, lookPos, up);
	}

	void Camera::OnDestroy()
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
	}

	auto Camera::GetProjMatrix() const -> const glm::mat4&
	{
		return matProj;
	}

	auto Camera::GetViewMatrix() const -> const glm::mat4&
	{
		return matView;
	}

	void Camera::SetDepth(int depth)
	{
		this->depth = depth;
		camera.SetPriority(depth);
	}

	void Camera::SetRenderTexture(render::RenderTexture& renderTexture)
	{
		this->renderTexture = &renderTexture;
		camera.SetRenderTexture(&renderTexture);
	}
	auto Camera::GetRenderTexture() const -> render::RenderTexture*
	{
		return renderTexture;
	}

	auto Camera::GetNative() -> render::Camera&
	{
		return camera;
	}

#ifdef SH_EDITOR
	void Camera::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (strcmp(prop.GetName(), "depth") == 0)
		{
			SetDepth(depth);
		}
	}
#endif
}