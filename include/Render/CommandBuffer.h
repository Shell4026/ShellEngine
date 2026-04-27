#pragma once

#include "Core/NonCopyable.h"

#include <array>
namespace sh::render
{
	class ComputeShader;
	class RenderTexture;
	class IBuffer;

	class CommandBuffer : public core::INonCopyable
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void Begin(bool bOnetime) = 0;
		virtual void End() = 0;

		virtual void Blit(RenderTexture& src, int x, int y, IBuffer& dst) = 0;
		virtual void Dispatch(const ComputeShader& shader, uint32_t x, uint32_t y, uint32_t z) = 0;
	};
}//namespace