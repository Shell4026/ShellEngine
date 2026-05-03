#pragma once
#include "Game/Export.h"
#include "Game/Vector.h"
#include "Game/Component/Component.h"

#include "Render/RenderData.h"
#include "Render/RenderTexture.h"

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

		SH_GAME_API auto ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray;
		SH_GAME_API auto ScreenPointToRayOrtho(const Vec2& mousePos) const -> phys::Ray;

		SH_GAME_API void SetPriority(int priority);
		SH_GAME_API void SetFov(float degree);
		            void SetRenderTexture(render::RenderTexture* renderTexture) { this->renderTexture = renderTexture; }
					void SetLookPos(const Vec3& pos) { lookPos = pos; }
		            void SetUpVector(const Vec3& up) { this->up = up; }
		            void SetProjection(Projection proj) { projection = proj; }
		            void SetNearPlane(float nearPlane) { this->nearPlane = nearPlane; }
		            void SetFarPlane(float farPlane) { this->farPlane = farPlane; }
		            void SetTag(const core::Name& tag) { renderData.tag = tag; }
		
		auto GetPriority() const -> int { return priority; }
		/// @brief 세로 시야각을 가져오는 함수.
		/// @return Degree로 반환한다.
		auto GetFov() const -> float { return fov; }
		/// @brief 가로 시야각을 가져오는 함수.
		/// @return Degree로 반환한다.
		auto GetFovx() const -> float { return fovx; }
		auto GetWidth() const -> float { return width; }
		auto GetHeight() const -> float { return height; }
		auto GetNearPlane() const -> float { return nearPlane; }
		auto GetFarPlane() const -> float { return farPlane; }
		auto GetUpVector() const -> const Vec3& { return up; }
		auto GetProjMatrix() const -> const glm::mat4& { return renderData.renderViewers.front().projMatrix; }
		auto GetViewMatrix() const -> const glm::mat4& { return renderData.renderViewers.front().viewMatrix; }
		auto GetLookPos() const -> const Vec3& { return lookPos; }
		auto GetTag() const -> const core::Name& { return renderData.tag; }
		auto GetProjection() const -> Projection { return projection; }
		auto GetRenderTexture() const -> render::RenderTexture* { return renderTexture; }
	protected:
		SH_RENDER_API virtual void UpdateViewMatrix();
		SH_RENDER_API virtual void UpdateProjMatrix();
	protected:
		PROPERTY(lookPos)
		Vec3 lookPos;
	private:
		PROPERTY(renderTexture)
		render::RenderTexture* renderTexture;

		PROPERTY(priority)
		int priority = 0;
		PROPERTY(fov)
		float fov = 60.f;
		float fovRadians = 0.f;
		float fovx = 0.f;
		PROPERTY(projection)
		Projection projection = Projection::Perspective;
		Vec3 up{ 0.f, 1.f, 0.f };
		PROPERTY(nearPlane)
		float nearPlane = 0.1f;
		PROPERTY(farPlane)
		float farPlane = 1000.f;
		float width = 1.f;
		float height = 1.f;

		render::RenderData renderData;
	};
}//namespace