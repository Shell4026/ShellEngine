#include "SObject.h"
#include "GarbageCollection.h"
#include "Observer.hpp"
#include "Util.h"

namespace sh::core
{
	SH_CORE_API SObject::SObject() :
		bPendingKill(false),
		uuid(UUID::Generate()), name("Unknown")
	{
		
	}
	SH_CORE_API SObject::SObject(const SObject& other) :
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		uuid(UUID::Generate()), name(other.name)
	{
	}
	SH_CORE_API SObject::SObject(SObject&& other) noexcept :
		bPendingKill(other.bPendingKill.load(std::memory_order::memory_order_relaxed)),
		uuid(std::move(other.uuid)), name(std::move(other.name)),
		onDestroy(std::move(other.onDestroy))
	{
		other.bPendingKill.store(true, std::memory_order_release);
		if (other.bMark.test_and_set(std::memory_order::memory_order_acquire))
			bMark.test_and_set(std::memory_order::memory_order_relaxed);
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
	auto SObject::operator new(std::size_t size, void* ptr) -> void*
	{
		return ::operator new(size, ptr);
	}
	void SObject::operator delete(void* ptr)
	{
		::operator delete(ptr);
	}
	void SObject::operator delete(void* ptr, std::size_t size)
	{
		::operator delete(ptr, size);
	}

	SH_CORE_API auto SObject::IsPendingKill() const -> bool
	{
		return bPendingKill.load(std::memory_order::memory_order_acquire);
	}

	SH_CORE_API void SObject::OnPropertyChanged(const reflection::Property& prop)
	{
	}

	SH_CORE_API void SObject::Destroy()
	{
		GarbageCollection::GetInstance()->RemoveRootSet(this);
		bPendingKill.store(true, std::memory_order::memory_order_release);
	}

	SH_CORE_API void SObject::OnDestroy()
	{
	}

	SH_CORE_API void SObject::SetName(std::string_view name)
	{
		this->name = Name{ name };
	}
	SH_CORE_API void SObject::SetName(const core::Name& name)
	{
		this->name = name;
	}
	SH_CORE_API auto SObject::GetName() const -> const Name&
	{
		return name;
	}

	SH_CORE_API auto SObject::SetUUID(const UUID& uuid) -> bool
	{
		SObjectManager& objManager = *SObjectManager::GetInstance();
		if (objManager.GetSObject(uuid) != nullptr)
			return false;

		objManager.UnRegisterSObject(this);
		this->uuid = uuid;
		RegisterToManager(this);
		return true;
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
		mainJson["type"] = stypeInfo->name.ToString();
		mainJson["uuid"] = uuid.ToString();
		mainJson["name"] = name.ToString();
		while (stypeInfo)
		{
			core::Json json{};
			for (auto& prop :stypeInfo->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;

				const reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				if (propType == core::reflection::GetType<int>())
					core::SerializeProperty(json, name, *prop->Get<int>(this));
				else if (propType == core::reflection::GetType<float>())
					core::SerializeProperty(json, name, *prop->Get<float>(this));
				else if (propType == core::reflection::GetType<std::string>())
					core::SerializeProperty(json, name, *prop->Get<std::string>(this));
				else if (propType == core::reflection::GetType<bool>())
					core::SerializeProperty(json, name, *prop->Get<bool>(this));
				else if (prop->isSObjectPointer)
				{
					auto ptr = prop->Get<SObject*>(this);
					if (core::IsValid(*ptr))
						core::SerializeProperty(json, name, *ptr);
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
		if (stypeInfo->name != json["type"].get<std::string>())
			return;

		if (json.contains("uuid"))
		{
			UUID newUUID{ json["uuid"].get<std::string>() };
			if (uuid != newUUID)
			{
				bool success = SetUUID(std::move(newUUID));
				assert(success);
			}
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
			for (auto& prop : stypeInfo->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				if (propType == core::reflection::GetType<int>())
				{
					core::DeserializeProperty(subJson, name, *prop->Get<int>(this));
					OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<float>())
				{
					core::DeserializeProperty(subJson, name, *prop->Get<float>(this));
					OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<std::string>())
				{
					core::DeserializeProperty(subJson, name, *prop->Get<std::string>(this));
					OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<bool>())
				{
					core::DeserializeProperty(subJson, name, *prop->Get<bool>(this));
					OnPropertyChanged(*prop.get());
				}
				else if (prop->isSObjectPointer)
				{
					core::DeserializeProperty(subJson, name, *prop->Get<SObject*>(this));
					OnPropertyChanged(*prop.get());
				}
			}
			stypeInfo = stypeInfo->GetSuper();
		}
	}
}