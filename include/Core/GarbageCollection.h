#pragma once
#include "Export.h"
#include "UUID.h"
#include "Singleton.hpp"

#include <cstdint>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <vector>
#include <queue>
#include <mutex>
#include <variant>
namespace sh::core::reflection
{
	class Property;
	class PropertyIterator;
}

namespace sh::core
{
	class SObject;
	
	/// @brief 가비지 컬렉터
	class GarbageCollection : public Singleton<GarbageCollection>
	{
		friend Singleton<GarbageCollection>;
	private:
		std::unordered_map<UUID, SObject*>& objs;
		std::unordered_map<SObject*, std::size_t> rootSetIdx;
		std::vector<SObject*> rootSets;
		int emptyRootSetCount = 0;

		struct IMap
		{
			virtual void Checking(GarbageCollection& gc) = 0;
			virtual void Unchecking(GarbageCollection& gc) = 0;
		};
		template<typename T, typename U>
		struct MapWrapper : IMap
		{
			std::variant<std::map<T, U>*, std::unordered_map<T, U>*> mapPtr;

			void Checking(GarbageCollection& gc) override
			{
				if (mapPtr.index() == 0)
				{
					std::map<T, U>* map = std::get<0>(mapPtr);
					for (auto it = map->begin(); it != map->end();)
					{
						if constexpr (std::is_convertible_v<T, const SObject*>)
						{
							const SObject* key = it->first;
							if (key->bPendingKill.load(std::memory_order::memory_order_acquire))
							{
								it = map->erase(it);
								continue;
							}
							std::queue<SObject*> bfs{};
							bfs.push(const_cast<SObject*>(key));
							while (!bfs.empty())
							{
								SObject* obj = bfs.front();
								bfs.pop();

								if (obj == nullptr)
									continue;
								if (obj->bMark.test_and_set(std::memory_order::memory_order_acquire))
									continue;

								gc.MarkProperties(obj, bfs);
							}
						}
						if constexpr (std::is_convertible_v<U, const SObject*>)
						{
							const SObject* value = it->second;
							if (value->bPendingKill.load(std::memory_order::memory_order_acquire))
							{
								it = map->erase(it);
								continue;
							}
							std::queue<SObject*> bfs{};
							bfs.push(const_cast<SObject*>(value));
							while (!bfs.empty())
							{
								SObject* obj = bfs.front();
								bfs.pop();

								if (obj == nullptr)
									continue;
								if (obj->bMark.test_and_set(std::memory_order::memory_order_acquire))
									continue;

								gc.MarkProperties(obj, bfs);
							}
						}
						++it;
					}
				}
				else
				{
					std::unordered_map<T, U>* map = std::get<1>(mapPtr);
					for (auto it = map->begin(); it != map->end();)
					{
						if constexpr (std::is_convertible_v<T, const SObject*>)
						{
							const SObject* key = it->first;
							if (key->bPendingKill.load(std::memory_order::memory_order_acquire))
							{
								it = map->erase(it);
								continue;
							}
							std::queue<SObject*> bfs{};
							bfs.push(const_cast<SObject*>(key));
							while (!bfs.empty())
							{
								SObject* obj = bfs.front();
								bfs.pop();

								if (obj == nullptr)
									continue;
								if (obj->bMark.test_and_set(std::memory_order::memory_order_acquire))
									continue;

								gc.MarkProperties(obj, bfs);
							}
						}
						if constexpr (std::is_convertible_v<U, const SObject*>)
						{
							const SObject* value = it->second;
							if (value->bPendingKill.load(std::memory_order::memory_order_acquire))
							{
								it = map->erase(it);
								continue;
							}
							std::queue<SObject*> bfs{};
							bfs.push(const_cast<SObject*>(value));
							while (!bfs.empty())
							{
								SObject* obj = bfs.front();
								bfs.pop();

								if (obj == nullptr)
									continue;
								if (obj->bMark.test_and_set(std::memory_order::memory_order_acquire))
									continue;

								gc.MarkProperties(obj, bfs);
							}
						}
						++it;
					}
				}
			}
			void Unchecking(GarbageCollection& gc) override
			{
				if (mapPtr.index() == 0)
				{
					std::map<T, U>* map = std::get<0>(mapPtr);
					for (auto& [key, value] : *map)
					{
						if constexpr (std::is_convertible_v<T, const SObject*>)
							gc.RemoveRootSet(key);
						if constexpr (std::is_convertible_v<U, const SObject*>)
							gc.RemoveRootSet(value);
					}
				}
				else
				{
					std::unordered_map<T, U>* map = std::get<1>(mapPtr);
					for (auto& [key, value] : *map)
					{
						if constexpr (std::is_convertible_v<T, const SObject*>)
							gc.RemoveRootSet(key);
						if constexpr (std::is_convertible_v<U, const SObject*>)
							gc.RemoveRootSet(value);
					}
				}
			}
		};
		struct MapWrapperDummy
		{
			void* vtable;
			void* ptr;
			void* dummy;
		};
		struct TrackingContainerInfo
		{
			enum class Type
			{
				Array,
				Vector,
				Set,
				HashSet,
				MapKey,
				MapValue,
				HashMapKey,
				HashMapValue
			} type;
			std::variant<std::size_t, MapWrapperDummy> data;
		};
		std::unordered_map<void*, TrackingContainerInfo> trackingContainers;
		using TrackingContainerIt = std::unordered_map<void*, TrackingContainerInfo>::iterator;

		std::mutex mu;

		uint32_t elapseTime = 0;
		uint32_t tick = 0;
		uint32_t updatePeriodTick = 1000;
		bool bContainerIteratorErased = false;
	public:
		static constexpr int DEFRAGMENT_ROOTSET_CAP = 32;
	private:
		/// @brief 중첩 컨테이너를 재귀로 순회하면서 SObject에 마킹 하는 함수
		/// @param bfs BFS용 큐
		/// @param parent 마킹이 시작된 오브젝트
		/// @param depth 현재 깊이
		/// @param maxDepth 최대 깊이
		/// @param it 넘길 반복자
		void ContainerMark(std::queue<SObject*>& bfs, SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it);
		void Mark(std::size_t start, std::size_t end);
		void MarkWithMultiThread();
		SH_CORE_API void MarkProperties(SObject* obj, std::queue<SObject*>& bfs);
		void CheckContainers(TrackingContainerIt start, TrackingContainerIt end);
		void CheckContainersWithMultiThread();
	protected:
		SH_CORE_API GarbageCollection();
	public:
		SH_CORE_API ~GarbageCollection();

		/// @brief 루트셋으로 지정하는 함수. 루트셋 객체는 참조하고 있는 객체가 없어도 메모리에서 유지된다.
		/// @param obj 루트셋으로 지정할 SObject 포인터
		SH_CORE_API void SetRootSet(SObject* obj);
		SH_CORE_API auto GetRootSet() const -> const std::vector<SObject*>&;
		/// @brief 해당 프레임마다 가비지 컬렉터를 수행한다.
		/// @param tick 목표 프레임
		SH_CORE_API void SetUpdateTick(uint32_t tick);

		/// @brief 루트셋에서 해당 객체를 제외하는 함수.
		/// @param obj SObject 포인터
		SH_CORE_API void RemoveRootSet(const SObject* obj);

		/// @brief 루트셋 배열의 빈공간을 조각 모음 하는 함수.
		/// @brief 쓰레기 수집이 시작되기 전 1프레임 전에 DEFRAGMENT_ROOTSET_CAP보다 빈 공간이 많아지면 실행 된다.
		SH_CORE_API void DefragmentRootSet();

		/// @brief GC를 갱신하며 지정된 시간이 흐르면 Collect()가 호출 된다.
		SH_CORE_API void Update();
		/// @brief 쓰레기 수집 시작
		SH_CORE_API void Collect();

		/// @brief GC에 등록된 오브젝트 개수를 확인하는 함수
		/// @return GC에 등록된 SObject개수
		SH_CORE_API auto GetObjectCount() const -> std::size_t;

		/// @brief 강제로 메모리를 해제 하는 함수. 주의해서 써야한다. 해당 포인터를 참조하고 있던 값은 변하지 않는다.
		/// @param obj SObject 포인터
		SH_CORE_API void ForceDelete(SObject* obj);

		/// @brief 이전에 GC를 수행하는데 걸린 시간(ms)을 반환 하는 함수
		SH_CORE_API auto GetElapsedTime() -> uint32_t;

		template<typename T, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::vector<T*>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), TrackingContainerInfo{ TrackingContainerInfo::Type::Vector, 0 });
		}
		template<typename T, std::size_t size, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::array<T*, size>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), TrackingContainerInfo{ TrackingContainerInfo::Type::Array, size });
		}
		template<typename T, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::set<T*>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), TrackingContainerInfo{ TrackingContainerInfo::Type::Set, 0 });
		}
		template<typename T, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::unordered_set<T*>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), TrackingContainerInfo{ TrackingContainerInfo::Type::HashSet, 0 });
		}
		template<typename T, typename U, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::map<T*, U>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			MapWrapper<T*, U> wrapper{};
			wrapper.mapPtr = &container;

			TrackingContainerInfo info{};
			info.type = TrackingContainerInfo::Type::MapKey;
			info.data = MapWrapperDummy{};

			std::memcpy(&info.data, &wrapper, sizeof(MapWrapper<T*, U>));
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), info);
		}
		template<typename T, typename U, typename = std::enable_if_t<std::is_base_of_v<SObject, U>>>
		void AddContainerTracking(std::map<T, U*>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			MapWrapper<T, U*> wrapper{};
			wrapper.mapPtr = &container;

			TrackingContainerInfo info{};
			info.type = TrackingContainerInfo::Type::MapValue;
			info.data = MapWrapperDummy{};

			std::memcpy(&info.data, &wrapper, sizeof(MapWrapper<T, U*>));
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), info);
		}
		template<typename T, typename U, typename = std::enable_if_t<std::is_base_of_v<SObject, T>>>
		void AddContainerTracking(std::unordered_map<T*, U>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			MapWrapper<T*, U> wrapper{};
			wrapper.mapPtr = &container;

			TrackingContainerInfo info{};
			info.type = TrackingContainerInfo::Type::HashMapKey;
			info.data = MapWrapperDummy{};

			std::memcpy(&info.data, &wrapper, sizeof(MapWrapper<T*, U>));
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), info);
		}
		template<typename T, typename U, typename = std::enable_if_t<std::is_base_of_v<SObject, U>>>
		void AddContainerTracking(std::unordered_map<T, U*>& container)
		{
			std::lock_guard<std::mutex> lock{ mu };
			MapWrapper<T, U*> wrapper{};
			wrapper.mapPtr = &container;

			TrackingContainerInfo info{};
			info.type = TrackingContainerInfo::Type::HashMapValue;
			info.data = MapWrapperDummy{};

			std::memcpy(&info.data, &wrapper, sizeof(MapWrapper<T, U*>));
			trackingContainers.insert_or_assign(reinterpret_cast<void*>(&container), info);
		}

		void RemoveContainerTracking(const void* containerPtr)
		{
			std::lock_guard<std::mutex> lock{ mu };
			auto it = trackingContainers.find(const_cast<void*>(containerPtr));
			if (it != trackingContainers.end())
				trackingContainers.erase(it);
		}
	};
}