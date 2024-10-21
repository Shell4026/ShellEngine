#pragma once

#include "Camera.h"
#include "Game/Vector.h"

#include "Render/RenderTexture.h"
#include "Render/IBuffer.h"

#include "Core/Observer.hpp"

#include <functional>
#include <queue>
#include <future>

namespace sh::game
{
	class PickingCamera : public Camera
	{
		COMPONENT(PickingCamera)
	private:
		PROPERTY(renderTex, core::PropertyOption::invisible)
		render::RenderTexture* renderTex = nullptr;
		PROPERTY(followCamera)
		Camera* followCamera = nullptr;
		
		int x = 0, y = 0;

		std::unique_ptr<render::IBuffer> buffer;

		uint8_t* pixels = nullptr;

		std::promise<void> renderAlreadyPromise;
		std::future<void> renderAlready;
		bool addTask = false;
	public:
		struct PixelData
		{
			uint8_t r, g, b, a;
		};
		core::Observer<true, uint8_t, uint8_t, uint8_t, uint8_t> pickingCallback;
	public:
		SH_GAME_API PickingCamera(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API void SetPickingPos(const Vec2& pos);
		SH_GAME_API void SetTextureSize(const Vec2& size);
		SH_GAME_API void SetFollowCamera(Camera* follow);
	};
}//namepspace