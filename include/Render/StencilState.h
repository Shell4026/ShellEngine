#pragma once
#include "Export.h"

#include "Core/ISerializable.h"

#include <cstdint>
#ifdef Always
#undef Always
#endif

namespace sh::render
{
	struct StencilState : core::ISerializable
	{
		enum class StencilOp
		{
			Keep = 0,
			Zero = 1,
			Replace = 2,
			IncrementClamp = 3,
			DecrementClamp = 4,
			Invert = 5,
			IncrementWrap = 6,
			DecrementWrap = 7
		};
		enum class CompareOp
		{
			Never = 0,
			Less = 1,
			Equal = 2,
			LessEqual = 3,
			Greater = 4,
			NotEqual = 5,
			GreaterEqual = 6,
			Always = 7,
		};

		uint32_t ref = 0;
		uint32_t compareMask = 255;
		uint32_t writeMask = 255;
		CompareOp compareOp = CompareOp::Always;
		StencilOp passOp = StencilOp::Keep;
		StencilOp failOp = StencilOp::Keep;
		StencilOp depthFailOp = StencilOp::Keep;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};
}//namespace