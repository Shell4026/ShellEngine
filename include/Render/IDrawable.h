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
		SH_RENDER_API virtual void Build(Material* mat, Mesh* mesh) = 0;
		SH_RENDER_API virtual void Update() = 0;
	};
}