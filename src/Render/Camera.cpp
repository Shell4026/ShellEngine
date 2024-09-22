#include "Camera.h"

namespace sh::render
{
	Camera::Camera() :
		renderTexture(nullptr), priority(0)
	{

	}
	void Camera::SetRenderTexture(RenderTexture* framebuffer)
	{
		this->renderTexture = framebuffer;
	}
	auto sh::render::Camera::GetRenderTexture() -> RenderTexture*
	{
		return renderTexture;
	}

	void Camera::SetPriority(int priority)
	{
		this->priority = priority;
	}
	auto Camera::GetPriority() const -> int
	{
		return priority;
	}
}//namespace render