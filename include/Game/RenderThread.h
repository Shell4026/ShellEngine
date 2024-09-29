#pragma once

#include "Export.h"

#include "Core/EngineThread.h"
#include "Core/Singleton.hpp"

namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class RenderThread :
		public core::EngineThread,
		public core::Singleton<RenderThread>
	{
		friend core::Singleton<RenderThread>;
	private:
		render::Renderer* renderer;
	private:
		SH_GAME_API RenderThread();
	public:
		SH_GAME_API void Init(render::Renderer& renderer);
	};
}//namespace