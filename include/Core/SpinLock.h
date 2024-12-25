#pragma once
#include "Export.h"

#include <atomic>
namespace sh::core
{
	class SpinLock
	{
	private:
		std::atomic_flag flag;
	public:
		SH_CORE_API SpinLock();

		SH_CORE_API void Lock();
		SH_CORE_API void UnLock();
	};
}//namespace