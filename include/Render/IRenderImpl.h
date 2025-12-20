#pragma once
#include "RenderData.h"
#include "CommandBuffer.h"

#include "Core/Name.h"

namespace sh::render
{
	class IRenderImpl
	{
	public:
		virtual ~IRenderImpl() = default;

		virtual void EmitBarrier(CommandBuffer& cmd, const std::vector<BarrierInfo>& barriers) = 0;
		virtual void RecordCommand(CommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList) = 0;
	};
}//namespace