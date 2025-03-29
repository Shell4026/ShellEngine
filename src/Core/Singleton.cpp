#include "Singleton.hpp"

#include <cstring>
#include <algorithm>
namespace sh::core
{
	std::mutex ISingleton::mu{};
	std::vector<ISingleton::InstanceInfo> ISingleton::instance{};

	auto ISingleton::CreateInstance(uint64_t hash, std::size_t size) -> Result
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = std::find_if(instance.begin(), instance.end(), [&](const InstanceInfo& info)
			{
				return info.hash == hash && info.size == size;
			}
		);
		if (it != instance.end())
			return Result{ it->ptr, false };

		void* ptr = ::operator new(size);
		std::memset(ptr, 0, size);
		instance.push_back(InstanceInfo{ hash, size, ptr });

		return Result{ ptr, true };
	}

	void ISingleton::DeleteInstance(uint64_t hash, std::size_t size)
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = std::find_if(instance.begin(), instance.end(), [&](const InstanceInfo& info)
			{
				return info.hash == hash && info.size == size;
			}
		);
		if (it == instance.end())
			return;

		void* instancePtr = it->ptr;
		::operator delete(instancePtr);
		instance.erase(it);
	}
}//namespace