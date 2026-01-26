#pragma once
#include "Game/Export.h"
#include "Game/Vector.h"
#include "Camera.h"

#include "Render/RenderTexture.h"
#include "Render/IBuffer.h"

#include "Core/Observer.hpp"

#include <future>

namespace sh::game
{
	class PickingCamera : public Camera
	{
		COMPONENT(PickingCamera)
	public:
		SH_GAME_API PickingCamera(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API void SetPickingPos(const Vec2& pos);
		SH_GAME_API void SetTextureSize(const Vec2& size);
		SH_GAME_API void SetFollowCamera(Camera* follow);
	public:
		struct PixelData
		{
			uint8_t r, g, b, a;
			SH_GAME_API operator uint32_t() const;
		};
		core::Observer<true, PixelData> pickingCallback;
	private:
		PROPERTY(renderTex, core::PropertyOption::invisible)
		render::RenderTexture* renderTex = nullptr;
		PROPERTY(followCamera)
		Camera* followCamera = nullptr;

		int x = 0, y = 0;

		uint8_t* pixels = nullptr;

		std::future<std::unique_ptr<render::IBuffer>> bufferFuture;

		bool bRequestRead = false;
	};
}//namepspace