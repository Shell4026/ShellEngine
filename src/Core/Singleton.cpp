#include "Singleton.hpp"

#include <cstring>
namespace sh::core
{
	std::mutex ISingleton::mu{};
	std::vector<std::pair<uint64_t, void*>> ISingleton::instance{};

	auto ISingleton::CreateInstance(uint64_t hash, std::size_t size) -> Result
	{
		std::lock_guard<std::mutex> lock{ mu };

		for (auto& [instanceHash, instancePtr] : instance)
		{
			if(instanceHash == hash)
				return Result{ instancePtr, false };
		}
		void* ptr = ::operator new(size);
		std::memset(ptr, 0, size);
		instance.push_back({ hash, ptr });

		return Result{ ptr, true };
	}

	void ISingleton::DeleteInstance(uint64_t hash)
	{
		std::lock_guard<std::mutex> lock{ mu };

		for (int i = 0; i < instance.size(); ++i)
		{
			uint64_t instanceHash = instance[i].first;
			void* instancePtr = instance[i].second;
			if (instanceHash == hash)
			{
				::operator delete(instancePtr);
				instance.erase(instance.begin() + i);
				return;
			}
		}
		return;
	}
}//namespace