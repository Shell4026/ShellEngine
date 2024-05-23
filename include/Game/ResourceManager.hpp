#pragma once

#include "Export.h"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/GC.h"

#include "Render/Renderer.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <string_view>
#include <functional>
#include <type_traits>

namespace sh::game
{
	template<typename T, 
		typename Condition = std::void_t<std::enable_if_t<std::is_base_of_v<sh::core::SObject, T>>>>
	class ResourceManager : public sh::core::INonCopyable
	{
	private:
		sh::core::GC& gc;
		sh::render::Renderer& renderer;

		std::unordered_map<std::string, std::unique_ptr<T>> resources;

		std::unordered_map<T*, std::unordered_map<void*, std::vector<std::function<void()>>>> resourceDestroyNotifies;
	public:
		ResourceManager(sh::core::GC& gc, sh::render::Renderer& renderer) :
			gc(gc), renderer(renderer)
		{}
		ResourceManager(ResourceManager&& other) noexcept :
			gc(other.gc), renderer(other.renderer),
			resources(std::move(other.resources)),
			resourceDestroyNotifies(std::move(other.resourceDestroyNotifies))
		{}
		~ResourceManager()
		{
			Clean();
		}

		void Clean()
		{
			for (auto& resourceNotify : resourceDestroyNotifies)
			{
				for (auto requester : resourceNotify.second)
				{
					for (auto& notify : requester.second)
						notify();
				}
			}
			resources.clear();
			resourceDestroyNotifies.clear();
		}

		auto AddResource(std::string_view _name, std::unique_ptr<T>&& resource) -> T*
		{
			std::string name{ _name };

			int idx = 0;
			auto it = resources.find(name);
			while (it != resources.end())
			{
				name += std::to_string(idx);
				it = resources.find(name);
			}

			resource->SetGC(gc);
			return resources.insert({ name, std::move(resource) }).first->second.get();
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

			auto resourcePtr = std::make_unique<T>(std::move(resource));
			resourcePtr->SetGC(gc);
			return resources.insert({ name, std::move(resourcePtr) }).first->second.get();
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

			auto resourcePtr = std::make_unique<T>(resource);
			resourcePtr->SetGC(gc);
			return resources.insert({ name, std::move(resourcePtr) }).first->second.get();
		}

		bool DestroyResource(std::string_view _name)
		{
			std::string name{ _name };
			auto it = resources.find(name);
			if (it == resources.end())
				return false;

			auto itNotify = resourceDestroyNotifies.find(it->second.get());
			if (itNotify != resourceDestroyNotifies.end())
			{
				for (auto requester : itNotify->second)
				{
					for(auto& func : requester.second)
						func();
				}
			}
			resourceDestroyNotifies.erase(itNotify);
			resources.erase(it);

			return true;
		}

		auto GetResource(std::string_view name) -> T*
		{
			auto it = resources.find(std::string{ name });
			if (it == resources.end())
				return nullptr;

			return it->second.get();
		}

		void RegisterDestroyNotify(void* requester, T* resource, const std::function<void()>& func)
		{
			auto it = resourceDestroyNotifies.find(resource);
			if (it == resourceDestroyNotifies.end())
			{
				std::unordered_map<void*, std::vector<std::function<void()>>> requesters;
				requesters.insert({ requester, std::vector<std::function<void()>>{func} });
				resourceDestroyNotifies.insert({ resource, std::move(requesters)});
			}
			else
			{
				auto requsterIt = it->second.find(requester);
				if (requsterIt == it->second.end())
				{
					it->second.insert({ requester, std::vector<std::function<void()>>{func} });
				}
				else
				{
					requsterIt->second.push_back(func);
				}
			}
		}
		void DestroyNotify(void* requester, T* resource)
		{
			auto it = resourceDestroyNotifies.find(resource);
			if (it == resourceDestroyNotifies.end())
				return;

			auto requesterIt = it->second.find(requester);
			if (requesterIt == it->second.end())
				return;

			requesterIt->second.clear();
		}
	};
}