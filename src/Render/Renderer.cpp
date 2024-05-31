#include "Renderer.h"

namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		apiType(api),
		viewportPos(0.f), viewportSize(100.f)
	{

	}
	void Renderer::PushDrawAble(IDrawable* drawable)
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