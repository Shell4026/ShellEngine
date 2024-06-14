#include "Renderer.h"

#include <cassert>
namespace sh::render
{
	Renderer::Renderer(RenderAPI api) :
		window(nullptr),
		apiType(api),
		viewportStart(0.f), viewportEnd(100.f)
	{

	}
	bool Renderer::Init(sh::window::Window& win)
	{
		window = &win;
		return true;
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

	auto Renderer::GetViewportStart() const -> const glm::vec2&
	{
		return viewportStart;
	}
	auto Renderer::GetViewportEnd() const -> const glm::vec2&
	{
		return viewportEnd;
	}
	auto Renderer::GetWindow() -> sh::window::Window&
	{
		assert(window);
		return *window;
	}
}