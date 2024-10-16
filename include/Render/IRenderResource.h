﻿#pragma once

#include "Export.h"

namespace sh::render
{
	class Renderer;

	/// @brief 렌더러 API에 맞는 객체를 생성 할 때 쓰는 인터페이스
	class IRenderResource
	{
	public:
		SH_RENDER_API virtual ~IRenderResource() = default;
		SH_RENDER_API virtual void Build(const Renderer& renderer) = 0;
	};
}//namespace