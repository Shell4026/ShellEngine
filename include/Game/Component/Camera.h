#pragma once

#include "Component.h"
#include "Game/Export.h"
#include "Game/Vector.h"

#include "Render/Renderer.h"
#include "Render/RenderTexture.h"
#include "Render/Framebuffer.h"
#include "Render/Camera.h"

#include "glm/mat4x4.hpp"

#include <memory>

namespace sh::phys
{
	class Ray;
}
namespace sh::game
{
	class Camera : public Component
	{
		COMPONENT(Camera)
	private:
		render::Camera camera;

		PROPERTY(renderTexture)
		render::RenderTexture* renderTexture;

		PROPERTY(depth)
		int depth;
		
		float fovRadians;
	protected:
		glm::mat4 matProj;
		glm::mat4 matView;

		PROPERTY(lookPos)
		Vec3 lookPos;
		Vec3 up;

		Vec2 screenSize;
	public:
		PROPERTY(fov)
		float fov;
		float nearPlane;
		float farPlane;

		const glm::mat4& worldToCameraMatrix;
	protected:
		inline void CalcMatrix();
	public:
		SH_GAME_API Camera(GameObject& owner);
		SH_GAME_API ~Camera();
		
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto GetProjMatrix() const -> const glm::mat4&;
		SH_GAME_API auto GetViewMatrix() const -> const glm::mat4&;

		SH_GAME_API void SetDepth(int depth);

		SH_GAME_API void SetRenderTexture(render::RenderTexture& renderTexture);
		SH_GAME_API auto GetRenderTexture() const -> render::RenderTexture*;

		SH_GAME_API auto GetNative() -> render::Camera&;

		SH_GAME_API auto ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray;

		SH_GAME_API void SetLookPos(const Vec3& pos);
		SH_GAME_API auto GetLookPos() const -> const Vec3&;
		SH_GAME_API void SetUpVector(const Vec3& up);
		SH_GAME_API auto GetUpVector() const -> const Vec3&;
#ifdef SH_EDITOR
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
#endif
	};
}