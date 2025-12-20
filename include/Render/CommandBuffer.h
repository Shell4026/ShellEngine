#pragma once

#include "Core/NonCopyable.h"

#include <array>
namespace sh::render
{
	class RenderTexture;
	class IBuffer;

	class CommandBuffer : public core::INonCopyable
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin(bool bOnetime) = 0;
		virtual void Blit(RenderTexture& src, int x, int y, IBuffer& dst) = 0;
		virtual void End() = 0;
	};
}//namespace