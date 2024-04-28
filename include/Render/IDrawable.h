#pragma once

#include "Export.h"

namespace sh::render
{
	class IDrawable
	{
		SH_RENDER_API virtual auto GetVertexCount() const -> int = 0;
	};
}