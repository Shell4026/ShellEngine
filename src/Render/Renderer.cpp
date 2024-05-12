#include "Renderer.h"

namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		apiType(api)
	{

	}
	void Renderer::PushDrawAble(Mesh* drawable)
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