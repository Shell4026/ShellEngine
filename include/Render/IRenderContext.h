#pragma once
#include "Export.h"

#include "Core/NonCopyable.h"

namespace sh::render
{
	enum class RenderAPI
	{
		OpenGL,
		Vulkan
	};
	class IRenderContext : public core::INonCopyable
	{
	public:
		virtual ~IRenderContext() = default;

		virtual void Init() = 0;
		virtual void Clean() = 0;
		virtual auto GetRenderAPIType() const -> RenderAPI = 0;
	};
}//namespace