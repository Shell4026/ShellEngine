#include "Renderer.h"

namespace sh::render
{
	void Renderer::PushDrawAble(const IDrawable* drawable)
	{
		if (drawable == nullptr)
			return;

		drawList.push(drawable);
	}

	void Renderer::ClearDrawList()
	{
		while (!drawList.empty())
			drawList.pop();
	}
}