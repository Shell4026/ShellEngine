#pragma once

#include "Export.h"

#include <memory>

namespace sh::render
{
	class Renderer;
	class ITextureBuffer;

	class TextureBufferFactory
	{
	public:
		static auto Create(const Renderer& renderer) -> std::unique_ptr<ITextureBuffer>;
	};
}//namespace