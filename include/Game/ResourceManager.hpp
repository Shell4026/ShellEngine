#pragma once

#include "Export.h"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/GarbageCollection.h"
#include "Core/SContainer.hpp"
#include "Core/Observer.hpp"

#include "Render/Renderer.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <string_view>
#include <functional>
#include <type_traits>

namespace sh::game
{
	template<typename T, typename KeyType = std::string,
		typename Condition = std::enable_if_t<std::is_base_of_v<sh::core::SObject, T>>>
	class ResourceManager : public sh::core::INonCopyable
	{
	private:
		core::GarbageCollection& gc;
		render::Renderer& renderer;

		std::unordered_map<KeyType, T*> resources;
	public:
		core::Observer<false, T*> onResourceDestroy;
	public:
		ResourceManager(sh::render::Renderer& renderer) :
			renderer(renderer), gc(*core::GarbageCollection::GetInstance()), onResourceDestroy()
		{
		}
		ResourceManager(ResourceManager&& other) noexcept :
			gc(other.gc),
			renderer(other.renderer),
			resources(std::move(other.resources)), onResourceDestroy()
		{

		
		}
		~ResourceManager()
		{
			Clean();
		}

		void Clean()
		{
			for (auto& [name, resPtr] :resources)
			{
				resPtr->Destroy();
				onResourceDestroy.Notify(resPtr);
			}
			resources.clear();
		}

		/// @brief 에셋을 추가한다.
		/// @param key 에셋 키
		/// @param resource 에셋 포인터
		/// @return 에셋 포인터
		auto AddResource(const KeyType& key, T* resource) -> T*
		{
			assert(resource);

			auto it = resources.find(key);
			if (it != resources.end())
				return nullptr;

			gc.SetRootSet(resource);
			return resources.insert({ key, resource }).first->second;
		}
		/// @brief 에셋을 이동시켜 추가한다.
		/// @param key 에셋 키
		/// @param resource 임시 에셋 객체
		/// @return 에셋 포인터
		auto AddResource(const KeyType& key, T&& resource) -> T*
		{
			auto it = resources.find(key);
			if (it != resources.end())
				return nullptr;

			auto resourcePtr = core::SObject::Create<T>(std::move(resource));

			gc.SetRootSet(resourcePtr);
			return resources.insert({ key, resourcePtr }).first->second;
		}
		/// @brief 에셋을 복사하여 추가한다.
		/// @param key 에셋 키
		/// @param resource 에셋 참조
		/// @return 에셋 포인터
		auto AddResource(const KeyType& key, const T& resource) -> T*
		{
			auto it = resources.find(key);
			if (it != resources.end())
				return nullptr;

			auto resourcePtr = core::SObject::Create<T>(resource);
			gc.SetRootSet(resourcePtr);
			return resources.insert({ key, resourcePtr }).first->second.get();
		}

		bool DestroyResource(const KeyType& key)
		{
			auto it = resources.find(key);
			if (it == resources.end())
				return false;

			T* resPtr = it->second;
			onResourceDestroy.Notify(resPtr);
			resPtr->Destroy();
			resources.erase(it);

			return true;
		}

		auto GetResource(const KeyType& key) -> T*
		{
			auto it = resources.find(key);
			if (it == resources.end())
				return nullptr;

			return it->second;
		}
	};
}