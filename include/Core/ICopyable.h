#pragma once

#include <memory>

namespace sh::core
{
	class ICopyable
	{
	public:
		virtual auto Clone() -> std::unique_ptr<ICopyable> = 0;
	};
}