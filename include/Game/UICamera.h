#pragma once
#include "Export.h"

#include "Render/Camera.h"

namespace sh::game
{
	class UICamera : public render::Camera
	{
	public:
		SH_GAME_API UICamera();
	protected:
		SH_GAME_API void UpdateViewMatrix() override;
		SH_GAME_API void UpdateProjMatrix() override;
	};
}//namespace