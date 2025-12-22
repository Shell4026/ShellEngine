#pragma once
#include "RenderData.h"
#include "CommandBuffer.h"
#include "IRenderThrMethod.h"

#include "Core/Name.h"

namespace sh::render
{
	template<>
	struct IRenderThrMethod<class IRenderImpl>
	{
		static void SetStoreImage(IRenderImpl& impl, bool bStore);
	};

	class IRenderImpl
	{
		friend IRenderThrMethod<class IRenderImpl>;
	public:
		virtual ~IRenderImpl() = default;

		virtual void EmitBarrier(CommandBuffer& cmd, const std::vector<BarrierInfo>& barriers) = 0;
		virtual void RecordCommand(CommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList) = 0;
	protected:
		virtual void SetStoreImage(bool bStore) = 0;
	};

	inline void IRenderThrMethod<class IRenderImpl>::SetStoreImage(IRenderImpl& impl, bool bStore)
	{
		impl.SetStoreImage(bStore);
	}
}//namespace