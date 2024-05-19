#pragma once

#include "Export.h"

#include <vector>

namespace sh::render
{
	class Mesh;
	class Material;

	class IDrawable
	{
	public:
		SH_RENDER_API virtual ~IDrawable() = default;
		SH_RENDER_API virtual void Build(Material* mat, Mesh* mesh) = 0;

		SH_RENDER_API virtual auto GetMaterial() const -> Material* = 0;
		SH_RENDER_API virtual auto GetMesh() const-> Mesh* = 0;
	};
}