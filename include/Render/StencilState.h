#pragma once

#include <cstdint>

namespace sh::render
{
	struct StencilState
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
			DecrementWrap = 7,
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

		uint32_t ref;
		uint32_t compareMask;
		uint32_t writeMask;
		CompareOp compareOp;
		StencilOp passOp;
		StencilOp failOp;
		StencilOp depthFailOp;
	};
}