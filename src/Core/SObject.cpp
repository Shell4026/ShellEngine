#include "PCH.h"
#include "SObject.h"

#include "GarbageCollection.h"
#include "Observer.hpp"
#include "Util.h"

#include <cstring>
#include <stack>

namespace sh::core
{
	SH_CORE_API SObject::SObject() :
		gc(GarbageCollection::GetInstance()),
		bPendingKill(false), bMark(false),
		uuid(UUID::Generate())
	{
		
	}
	SH_CORE_API SObject::SObject(const SObject& other) :
		gc(other.gc), 
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		bMark(other.bMark),
		uuid(other.uuid), name(other.name)
	{
	}
	SH_CORE_API SObject::SObject(SObject&& other) noexcept :
		gc(other.gc),
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		bMark(other.bMark),
		uuid(other.uuid), name(std::move(other.name))
	{
		other.bPendingKill.store(true, std::memory_order_release);
		other.bMark = false;
	}
	SH_CORE_API SObject::~SObject()
	{
		onDestroy.Notify(this);
		SObjectManager::GetInstance()->UnRegisterSObject(this);
	}

	SH_CORE_API void SObject::RegisterToManager(SObject* ptr)
	{
		SObjectManager::GetInstance()->RegisterSObject(ptr);
	}
	auto SObject::operator new(std::size_t size) -> void*
	{
		return ::operator new(size);
	}
	void SObject::operator delete(void* ptr)
	{
		::operator delete(ptr);
	}

	SH_CORE_API auto SObject::IsPendingKill() const -> bool
	{
		return bPendingKill.load(std::memory_order::memory_order_acquire);
	}
	SH_CORE_API auto SObject::IsMark() const -> bool
	{
		return bMark;
	}

	SH_CORE_API void SObject::OnPropertyChanged(const reflection::Property& prop)
	{
	}

	SH_CORE_API void SObject::Destroy()
	{
		bPendingKill.store(true, std::memory_order::memory_order_release);
		gc->RemoveRootSet(this);
	}

	SH_CORE_API void SObject::OnDestroy()
	{
	}

	SH_CORE_API void SObject::SetName(std::string_view name)
	{
		this->name = name;
	}
	SH_CORE_API auto SObject::GetName() const -> const std::string&
	{
		return name;
	}

	SH_CORE_API void SObject::SetUUID(const UUID& uuid)
	{
		SObjectManager::GetInstance()->UnRegisterSObject(this);
		this->uuid = uuid;
		RegisterToManager(this);
	}
	SH_CORE_API auto SObject::GetUUID() const -> const UUID&
	{
		return uuid;
	}

	SH_CORE_API auto SObject::Serialize() const -> core::Json
	{
		const reflection::STypeInfo* stypeInfo = &GetType();
		core::Json mainJson;
		mainJson["version"] = 1;
		mainJson["type"] = stypeInfo->name;
		mainJson["uuid"] = uuid.ToString();
		mainJson["name"] = name;
		while (stypeInfo)
		{
			core::Json json{};
			for (auto& [name, prop] :stypeInfo->GetProperties())
			{
				const reflection::TypeInfo& propType = prop.type;
				if (propType == core::reflection::GetType<int>())
					core::SerializeProperty(json, name, *prop.Get<int>(this));
				else if (propType == core::reflection::GetType<float>())
					core::SerializeProperty(json, name, *prop.Get<float>(this));
				else if (propType == core::reflection::GetType<std::string>())
					core::SerializeProperty(json, name, *prop.Get<std::string>(this));
				else if (propType == core::reflection::GetType<bool>())
					core::SerializeProperty(json, name, *prop.Get<bool>(this));
				else if (prop.isSObjectPointer)
				{
					SObject* ptr = *prop.Get<SObject*>(this);
					if (core::IsValid(ptr))
						core::SerializeProperty(json, name, ptr);
				}
			}
			if (!json.empty())
				mainJson[stypeInfo->name] = json;
			stypeInfo = stypeInfo->GetSuper();
		}
		return mainJson;
	}
	SH_CORE_API void SObject::Deserialize(const nlohmann::json& json)
	{
		const reflection::STypeInfo* stypeInfo = &GetType();
		if (json["type"].get<std::string>() != stypeInfo->name)
			return;

		if (json.contains("uuid"))
		{
			SetUUID(UUID{ json["uuid"].get<std::string>() });
		}
		if (json.contains("name"))
			SetName(json["name"].get<std::string>());

		while (stypeInfo)
		{
			if (!json.contains(stypeInfo->name))
			{
				stypeInfo = stypeInfo->GetSuper();
				continue;
			}
			core::Json subJson = json[stypeInfo->name];
			for (auto& [name, prop] : stypeInfo->GetProperties())
			{
				const reflection::TypeInfo& propType = prop.type;
				if (propType == core::reflection::GetType<int>())
					core::DeserializeProperty(subJson, name, *prop.Get<int>(this));
				else if (propType == core::reflection::GetType<float>())
					core::DeserializeProperty(subJson, name, *prop.Get<float>(this));
				else if (propType == core::reflection::GetType<std::string>())
					core::DeserializeProperty(subJson, name, *prop.Get<std::string>(this));
				else if (propType == core::reflection::GetType<bool>())
					core::DeserializeProperty(subJson, name, *prop.Get<bool>(this));
				else if (prop.isSObjectPointer)
					core::DeserializeProperty(subJson, name, *prop.Get<SObject*>(this));
				OnPropertyChanged(prop);
			}
			stypeInfo = stypeInfo->GetSuper();
		}
	}
}