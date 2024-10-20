#include "PCH.h"
#include "Component/Camera.h"
#include "GameObject.h"

#include "Physics/Ray.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cstdint>

namespace sh::game
{
	Camera::Camera(GameObject& owner) :
		Component(owner),

		worldToCameraMatrix(matView),
		matProj(), matView(),
		fov(60.f), fovRadians(glm::radians(60.f)), nearPlane(0.1f), farPlane(1000.f),
		camera(), depth(0), lookPos({ 0.f, 0.f, 0.f }), up({ 0.f, 1.f, 0.f }),

		renderTexture(nullptr)
	{
	}
	Camera::~Camera()
	{
	}
	
	void Camera::Awake()
	{
		Super::Awake();
		gameObject.world.RegisterCamera(this);
	}
	void Camera::Start()
	{
	}
	void Camera::BeginUpdate()
	{
		if (renderTexture == nullptr)
			screenSize = gameObject.world.renderer.GetViewportEnd() - gameObject.world.renderer.GetViewportStart();
		else
			screenSize = renderTexture->GetSize();
		
		CalcMatrix();
	}
	void Camera::CalcMatrix()
	{
		fovRadians = glm::radians(fov);

		matProj = glm::perspectiveFov(fovRadians,
			static_cast<float>(screenSize.x),
			static_cast<float>(screenSize.y),
			nearPlane, farPlane);

		matView = glm::lookAt(glm::vec3{ gameObject.transform->position }, glm::vec3{ lookPos }, glm::vec3{ up });
	}

	void Camera::OnDestroy()
	{
		if (gameObject.world.GetMainCamera() == this)
		{
			//다음에 추가된 카메라를 메인 카메라로 한다.
			for (auto& cam : gameObject.world.GetCameras())
			{
				if (cam == this)
					continue;

				gameObject.world.SetMainCamera(cam);
				break;
			}
		}
		gameObject.world.UnRegisterCamera(this);
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

	auto Camera::ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray
	{
		float aspect = screenSize.x / screenSize.y;

		float ndcX = 2.f * mousePos.x / screenSize.x - 1.0f;
		float ndcY = 1.0f -2.f * mousePos.y / screenSize.y;

		glm::vec4 camCoord{ 0.f, 0.f, 0.f, 1.f };

		camCoord.x = aspect * ndcX;
		camCoord.y = ndcY;
		camCoord.z = -1.f / glm::tan(fovRadians / 2.f);

		glm::vec3 worldCoord{ glm::inverse(matView) * camCoord };
		glm::vec3 dir = glm::normalize(worldCoord - glm::vec3{ gameObject.transform->position });

		return phys::Ray(gameObject.transform->position, dir);
	}

	SH_GAME_API void Camera::SetLookPos(const Vec3& pos)
	{
		lookPos = pos;
	}
	SH_GAME_API auto Camera::GetLookPos() const -> const Vec3&
	{
		return lookPos;
	}
	SH_GAME_API void Camera::SetUpVector(const Vec3& up)
	{
		this->up = up;
	}
	SH_GAME_API auto Camera::GetUpVector() const -> const Vec3&
	{
		return up;
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