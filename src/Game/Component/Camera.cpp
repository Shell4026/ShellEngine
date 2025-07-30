#include "Component/Camera.h"
#include "GameObject.h"

#include "Render/IRenderContext.h"

#include "Physics/Ray.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cstdint>

namespace sh::game
{
	Camera::Camera(GameObject& owner) :
		Component(owner),

		fov(60.f),
		camera(), depth(0),

		renderTexture(nullptr)
	{
		owner.world.RegisterCamera(this);
		camera.SetNearPlane(0.1f);
		camera.SetFarPlane(1000.f);
		canPlayInEditor = true;
	}
	Camera::~Camera()
	{
	}
	
	SH_GAME_API void Camera::BeginUpdate()
	{
		if (renderTexture == nullptr)
		{
			glm::vec2 size = gameObject.world.renderer.GetContext()->GetViewportEnd() - gameObject.world.renderer.GetContext()->GetViewportStart();
			camera.SetWidth(size.x);
			camera.SetHeight(size.y);
		}
		else
		{
			glm::vec2 size = renderTexture->GetSize();
			camera.SetWidth(size.x);
			camera.SetHeight(size.y);
		}
		camera.SetPos(gameObject.transform->GetWorldPosition());
		camera.SetLookPos(lookPos);
		camera.UpdateMatrix();
	}
	SH_GAME_API void Camera::OnDestroy()
	{
		if (gameObject.world.GetMainCamera() == this)
		{
			gameObject.world.SetMainCamera(nullptr);
		}
		gameObject.world.UnRegisterCamera(this);
		Super::OnDestroy();
	}
	SH_GAME_API void Camera::SetActive(bool b)
	{
		Super::SetActive(b);
		camera.SetActive(b);
	}

	SH_GAME_API auto Camera::GetProjMatrix() const -> const glm::mat4&
	{
		return camera.GetProjMatrix(core::ThreadType::Game);
	}

	SH_GAME_API auto Camera::GetViewMatrix() const -> const glm::mat4&
	{
		return camera.GetViewMatrix(core::ThreadType::Game);
	}

	SH_GAME_API void Camera::SetDepth(int depth)
	{
		this->depth = depth;
		camera.SetPriority(depth);
	}
	SH_GAME_API void Camera::SetFov(float degree)
	{
		camera.SetFov(degree);
	}

	SH_GAME_API void Camera::SetRenderTexture(render::RenderTexture* renderTexture)
	{
		this->renderTexture = renderTexture;
		camera.SetRenderTexture(renderTexture);
	}
	SH_GAME_API auto Camera::GetRenderTexture() const -> render::RenderTexture*
	{
		return renderTexture;
	}

	SH_GAME_API auto Camera::GetNative() -> render::Camera&
	{
		return camera;
	}

	SH_GAME_API auto Camera::GetNative() const -> const render::Camera&
	{
		return camera;
	}

	SH_GAME_API auto Camera::ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray
	{
		float w = camera.GetWidth(core::ThreadType::Game);
		float h = camera.GetHeight(core::ThreadType::Game);
		float aspect = w / h;

		float ndcX = 2.f * mousePos.x / w - 1.0f;
		float ndcY = 1.0f - 2.f * mousePos.y / h;

		glm::vec4 camCoord{ 0.f, 0.f, 0.f, 1.f };

		camCoord.x = aspect * ndcX;
		camCoord.y = ndcY;
		camCoord.z = -1.f / glm::tan(camera.GetFovRadian() / 2.f);

		glm::vec3 worldCoord{ glm::inverse(camera.GetViewMatrix(core::ThreadType::Game)) * camCoord };
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
		camera.SetUpVector(up);
	}
	SH_GAME_API auto Camera::GetUpVector() const -> Vec3
	{
		return camera.GetUpVector(core::ThreadType::Game);
	}
	SH_GAME_API void Camera::SetWidth(float width)
	{
		camera.SetWidth(width);
	}
	SH_GAME_API auto Camera::GetWidth() const -> float
	{
		return camera.GetWidth(core::ThreadType::Game);
	}
	SH_GAME_API void Camera::SetHeight(float height)
	{
		camera.SetHeight(height);
	}
	SH_GAME_API auto Camera::GetHeight() const -> float
	{
		return camera.GetHeight(core::ThreadType::Game);
	}

	SH_GAME_API void Camera::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "depth")
		{
			SetDepth(depth);
		}
		else if (prop.GetName() == "fov")
		{
			SetFov(fov);
		}
	}
}