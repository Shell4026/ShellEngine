#pragma once
#include "Export.h"
#include "CommandBuffer.h"
#include "IRenderImpl.h"

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
		virtual void Clear() = 0;
		virtual auto GetRenderAPIType() const -> RenderAPI = 0;

		virtual auto AllocateCommandBuffer() -> CommandBuffer* = 0;
		virtual void DeallocateCommandBuffer(CommandBuffer& cmd) = 0;

		virtual void SetViewport(const glm::vec2& start, const glm::vec2& end) = 0;
		virtual auto GetViewportStart() const -> const glm::vec2& = 0;
		virtual auto GetViewportEnd() const -> const glm::vec2& = 0;
		virtual auto GetRenderImpl() const -> IRenderImpl& = 0;
	};
}//namespace