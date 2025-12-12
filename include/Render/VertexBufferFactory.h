#pragma once
#include "Export.h"
#include "IVertexBuffer.h"

#include <memory>

namespace sh::render
{
	class IRenderContext;
	class Mesh;

	/// @brief 렌더러 API에 맞는 버텍스 버퍼를 생성하는 클래스
	class VertexBufferFactory
	{
	public:
		SH_RENDER_API static auto Create(const IRenderContext& v, const Mesh& mesh) -> std::unique_ptr<IVertexBuffer>;
	};
}//namespace