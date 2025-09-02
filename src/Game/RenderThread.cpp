#include "RenderThread.h"

#include "Core/Logger.h"

#include "Render/Renderer.h"

namespace sh::game
{
	RenderThread::RenderThread() :
		renderer(nullptr)
	{
	}

	void RenderThread::Init(render::Renderer& renderer)
	{
		this->renderer = &renderer;

		AddTask([&]
			{
				this->renderer->Render();
			}
		);
	}
}//namespace