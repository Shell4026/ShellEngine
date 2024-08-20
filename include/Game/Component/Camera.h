#pragma once

#include "Component.h"
#include "Game/Export.h"

#include "Render/Renderer.h"
#include "Render/RenderTexture.h"
#include "Render/Framebuffer.h"

#include "glm/mat4x4.hpp"

#include <memory>

namespace sh::game
{
	class Camera : public Component
	{
		SCLASS(Camera)
	private:
		glm::mat4 matProj;
		glm::mat4 matView;

		PROPERTY(cameraHandle, const)
		uint32_t cameraHandle;

		PROPERTY(depth)
		int depth;
	public:
		float fov;
		float nearPlane;
		float farPlane;

		PROPERTY(renderTexture)
		render::RenderTexture* renderTexture;

		const glm::mat4& worldToCameraMatrix;
	public:
		SH_GAME_API Camera();
		SH_GAME_API ~Camera();
		
		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;

		SH_GAME_API auto GetProjMatrix() const -> const glm::mat4&;
		SH_GAME_API auto GetViewMatrix() const -> const glm::mat4&;

		SH_GAME_API auto GetCameraHandle() const -> uint32_t;

		SH_GAME_API void SetDepth(int depth);

#ifdef SH_EDITOR
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
#endif
	};
}