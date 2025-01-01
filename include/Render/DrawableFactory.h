#pragma once

#include "Export.h"

namespace sh::render
{
	class IRenderContext;
	class IDrawable;

	/// @brief 렌더러 API에 따라 맞는 drawable을 생성하는 클래스.
	class DrawableFactory
	{
	public:
		SH_RENDER_API static auto Create(const IRenderContext& context) -> IDrawable*;
	};
}//namespace