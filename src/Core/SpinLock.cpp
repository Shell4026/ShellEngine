#include "PCH.h"
#include "SpinLock.h"

#include <thread>

namespace sh::core
{
	SpinLock::SpinLock()
	{
	}
	SH_CORE_API void SpinLock::Lock()
	{
		while (flag.test_and_set(std::memory_order::memory_order_acquire))
		{
			std::this_thread::yield();
		}
	}
	SH_CORE_API void SpinLock::UnLock()
	{
		flag.clear(std::memory_order::memory_order_release);
	}
}//namespace