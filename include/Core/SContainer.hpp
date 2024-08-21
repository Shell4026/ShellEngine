#pragma once

#include "Memory/SAllocator.hpp"

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace sh::core
{
	template<typename T>
	using SVector = std::vector<T>;

	template<typename T, std::size_t defaultSize = 32, typename _Pr = std::less<T>>
	using SSet = std::set<T, _Pr, memory::SAllocator<T, defaultSize>>;

	template<typename T, std::size_t defaultSize = 32, typename _hasher = std::hash<T>, typename _Keyeq = std::equal_to<T>>
	using SHashSet = std::unordered_set<T, _hasher, _Keyeq, memory::SAllocator<T, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 32, typename _Pr = std::less<KeyT>>
	using SMap = std::map<KeyT, T, _Pr, memory::SAllocator<std::pair<const KeyT, T>, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 32, typename _hasher = std::hash<KeyT>, typename _Keyeq = std::equal_to<KeyT>>
	using SHashMap = std::unordered_map<KeyT, T, _hasher, _Keyeq, memory::SAllocator<std::pair<const KeyT, T>, defaultSize>>;
}//namespace