#pragma once

#include "Export.h"
#include "Material.h"

#include <vector>

namespace sh::render
{
	class IDrawable
	{
	public:
		SH_RENDER_API virtual auto GetVertexCount() const -> int = 0;
		SH_RENDER_API virtual auto GetMaterial(int id) -> Material* = 0;
		SH_RENDER_API virtual auto GetMaterials()-> std::vector<Material*>& = 0;
	};
}