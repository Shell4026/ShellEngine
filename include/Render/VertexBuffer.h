﻿#pragma once

#include "Export.h"

#include <cstdint>
#include <string_view>

namespace sh::render
{
	class Mesh;

	class VertexBuffer
	{
	private:

	public:
		SH_RENDER_API virtual ~VertexBuffer() = default;

		SH_RENDER_API virtual void Create(const Mesh& mesh) = 0;
		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API virtual void Bind() = 0;
	};
}