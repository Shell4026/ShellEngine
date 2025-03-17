#pragma once
#include "Export.h"
#include "UniformStructLayout.h"

#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"

namespace sh::render
{
	class IRenderContext;
	class ShaderPass;
	class Texture;
	class IBuffer;

	class IUniformBuffer : public core::INonCopyable
	{
	public:
		SH_RENDER_API virtual ~IUniformBuffer() = default;

		SH_RENDER_API virtual void Create(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Type type) = 0;
		SH_RENDER_API virtual void Clear() = 0;
		SH_RENDER_API virtual void Link(uint32_t binding, const IBuffer& buffer, std::size_t bufferSize = 0) = 0;
		SH_RENDER_API virtual void Link(uint32_t binding, const Texture& texture) = 0;
	};
}//namespace