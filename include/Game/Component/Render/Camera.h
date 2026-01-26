#pragma once
#include "Game/Export.h"
#include "Game/Vector.h"
#include "Game/Component/Component.h"

#include "Render/Renderer.h"
#include "Render/RenderTexture.h"
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
	public:
		enum class Projection
		{
			Perspective,
			Orthographic
		};
	public:
		SH_GAME_API Camera(GameObject& owner);
		SH_GAME_API ~Camera();
		
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API void SetActive(bool b) override;

		SH_GAME_API auto GetProjMatrix() const -> const glm::mat4&;
		SH_GAME_API auto GetViewMatrix() const -> const glm::mat4&;

		SH_GAME_API void SetDepth(int depth);
		SH_GAME_API auto GetDepth() const -> int { return depth; }
		SH_GAME_API void SetFov(float degree);
		
		/// @brief 세로 시야각을 가져오는 함수.
		/// @return Degree로 반환한다.
		SH_GAME_API auto GetFov() const -> float { return fov; }
		/// @brief 가로 시야각을 가져오는 함수.
		/// @return Degree로 반환한다.
		SH_GAME_API auto GetFovx() const -> float { return fovx; }

		SH_GAME_API void SetRenderTexture(render::RenderTexture* renderTexture);
		SH_GAME_API auto GetRenderTexture() const -> render::RenderTexture*;

		SH_GAME_API auto GetNative() -> render::Camera&;
		SH_GAME_API auto GetNative() const -> const render::Camera&;

		SH_GAME_API auto ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray;
		SH_GAME_API auto ScreenPointToRayOrtho(const Vec2& mousePos) const -> phys::Ray;

		SH_GAME_API void SetLookPos(const Vec3& pos);
		SH_GAME_API auto GetLookPos() const -> const Vec3&;
		SH_GAME_API void SetUpVector(const Vec3& up);
		SH_GAME_API auto GetUpVector() const -> Vec3;

		SH_GAME_API void SetWidth(float width);
		SH_GAME_API auto GetWidth() const -> float;
		SH_GAME_API void SetHeight(float height);
		SH_GAME_API auto GetHeight() const -> float;

		SH_GAME_API void SetProjection(Projection proj);
		SH_GAME_API auto GetProjection() const -> Projection;

		SH_GAME_API void SetNearPlane(float near);
		SH_GAME_API auto GetNearPlane() const -> float;
		SH_GAME_API void SetFarPlane(float near);
		SH_GAME_API auto GetFarPlane() const -> float;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	protected:
		render::Camera camera;

		PROPERTY(lookPos)
		Vec3 lookPos;
	private:
		PROPERTY(renderTexture)
		render::RenderTexture* renderTexture;

		PROPERTY(depth)
		int depth;
		PROPERTY(fov)
		float fov;
		float fovx;
		PROPERTY(projection)
		Projection projection = Projection::Perspective;
	};
}