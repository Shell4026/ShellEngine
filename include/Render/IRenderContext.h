#pragma once
#include "Export.h"

#include "Core/NonCopyable.h"

#include <glm/vec2.hpp>

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

		virtual void SetViewport(const glm::vec2& start, const glm::vec2& end) = 0;
		virtual auto GetViewportStart() const -> const glm::vec2& = 0;
		virtual auto GetViewportEnd() const -> const glm::vec2& = 0;
	};
}//namespace