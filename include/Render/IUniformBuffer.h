#pragma once

#include "Export.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"

namespace sh::render
{
	class IRenderContext;
	class Shader;
	class Texture;
	class IBuffer;

	class IUniformBuffer : public core::INonCopyable
	{
	public:
		SH_RENDER_API virtual ~IUniformBuffer() = default;

		SH_RENDER_API virtual void Create(const IRenderContext& context, const Shader& shader, uint32_t type) = 0;
		SH_RENDER_API virtual void Clean() = 0;
		SH_RENDER_API virtual void Update(uint32_t binding, const IBuffer& buffer) = 0;
		SH_RENDER_API virtual void Update(uint32_t binding, const Texture& texture) = 0;
	};
}//namespace