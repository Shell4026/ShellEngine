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
	template<typename T, 
		typename Condition = std::enable_if_t<std::is_base_of_v<sh::core::SObject, T>>>
	class ResourceManager : public sh::core::INonCopyable
	{
	private:
		core::GarbageCollection& gc;
		render::Renderer& renderer;

		core::SHashMap<std::string, T*> resources;

		core::Observer<T*> onResourceDestroy;
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

		auto AddResource(std::string_view _name, T* resource) -> T*
		{
			assert(resource);
			std::string name{ _name };

			int idx = 0;
			auto it = resources.find(name);
			while (it != resources.end())
			{
				name += std::to_string(idx);
				it = resources.find(name);
			}
#if SH_EDITOR
			resource->editorName = name;
#endif
			gc.SetRootSet(resource);
			return resources.insert({ std::move(name), resource }).first->second;
		}
		auto AddResource(std::string_view _name, T&& resource) -> T*
		{
			std::string name{ _name };

			int idx = 0;
			auto it = resources.find(name);
			while (it != resources.end())
			{
				name += std::to_string(idx);
				it = resources.find(name);
			}

			auto resourcePtr = core::SObject::Create<T>(std::move(resource));
#if SH_EDITOR
			resourcePtr->editorName = name;
#endif
			gc.SetRootSet(resourcePtr);
			return resources.insert({ std::move(name), resourcePtr }).first->second;
		}
		auto AddResource(std::string_view _name, const T& resource) -> T*
		{
			std::string name{ _name };

			int idx = 0;
			auto it = resources.find(name);
			while (it != resources.end())
			{
				name += std::to_string(idx);
				it = resources.find(name);
			}

			auto resourcePtr = core::SObject::Create<T>(resource);
#if SH_EDITOR
			resourcePtr->editorName = name;
#endif
			gc.SetRootSet(resourcePtr);
			return resources.insert({ std::move(name), resourcePtr }).first->second.get();
		}

		bool DestroyResource(std::string_view _name)
		{
			std::string name{ _name };
			auto it = resources.find(name);
			if (it == resources.end())
				return false;

			T* resPtr = it->second;
			onResourceDestroy.Notify(resPtr);
			resPtr->Destroy();
			resources.erase(it);

			return true;
		}

		auto GetResource(std::string_view name) -> T*
		{
			auto it = resources.find(std::string{ name });
			if (it == resources.end())
				return nullptr;

			return it->second;
		}
	};
}