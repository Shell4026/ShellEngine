#include "SObject.h"
#include "GarbageCollection.h"
#include "Observer.hpp"
#include "Util.h"
#include "AssetResolver.h"

namespace sh::core
{
	template<>
	SH_CORE_API auto DeserializeProperty(const core::Json& json, const std::string& key, SObject*& value) -> bool
	{
		if (json.contains(key))
		{
			std::string uuid = json[key].get<std::string>();
			auto objectManager = SObjectManager::GetInstance();
			SObject* ptr = objectManager->GetSObject(UUID{ uuid });
			if (ptr == nullptr)
			{
				auto& resolver = AssetResolverRegistry::GetResolver();
				if (resolver)
					ptr = resolver(UUID{ uuid });
				else
					return false;
			}
			value = ptr;
			return true;
		}
		return false;
	}

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
		if (bPendingKill.exchange(true, std::memory_order::memory_order_acq_rel))
			return;
		GarbageCollection& gc = *GarbageCollection::GetInstance();
		gc.RemoveRootSet(this);

		OnDestroy();
	}

	SH_CORE_API void SObject::OnDestroy()
	{
		GarbageCollection& gc = *GarbageCollection::GetInstance();
		gc.AddToPendingKillList(this);
		onDestroy.Notify(this);
	}

	SH_CORE_API void SObject::SetName(std::string_view name)
	{
		this->name = Name{ name };
	}
	SH_CORE_API void SObject::SetName(const core::Name& name)
	{
		this->name = name;
	}
	SH_CORE_API void SObject::SetName(core::Name&& name)
	{
		this->name = std::move(name);
	}
	SH_CORE_API auto SObject::GetName() const -> const Name&
	{
		return name;
	}

	SH_CORE_API auto SObject::SetUUID(const UUID& uuid) -> bool
	{
		SObjectManager& objManager = *SObjectManager::GetInstance();
		if (objManager.GetSObject(uuid) != nullptr)
		{
			SH_ERROR_FORMAT("UUID({}) already exists!", uuid.ToString());
			return false;
		}

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

				if (propType == core::reflection::GetType<int>() || prop->isEnum)
					core::SerializeProperty(json, name, *prop->Get<int>(*this));
				else if (propType == core::reflection::GetType<uint32_t>())
					core::SerializeProperty(json, name, *prop->Get<uint32_t>(*this));
				else if (propType == core::reflection::GetType<int64_t>())
					core::SerializeProperty(json, name, *prop->Get<int64_t>(*this));
				else if (propType == core::reflection::GetType<uint64_t>())
					core::SerializeProperty(json, name, *prop->Get<uint64_t>(*this));
				else if (propType == core::reflection::GetType<float>())
					core::SerializeProperty(json, name, *prop->Get<float>(*this));
				else if (propType == core::reflection::GetType<double>())
					core::SerializeProperty(json, name, *prop->Get<double>(*this));
				else if (propType == core::reflection::GetType<char>())
					core::SerializeProperty(json, name, *prop->Get<char>(*this));
				else if (propType == core::reflection::GetType<std::string>())
					core::SerializeProperty(json, name, *prop->Get<std::string>(*this));
				else if (propType == core::reflection::GetType<bool>())
					core::SerializeProperty(json, name, *prop->Get<bool>(*this));
				else if (prop->isSObjectPointer)
				{
					auto ptr = prop->Get<SObject*>(*this);

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
			const core::Json& subJson = json[stypeInfo->name];
			for (auto& prop : stypeInfo->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				if (propType == core::reflection::GetType<int>() || prop->isEnum)
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<int>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<uint32_t>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<uint32_t>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<int64_t>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<int64_t>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<uint64_t>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<uint64_t>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<float>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<float>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<double>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<double>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<std::string>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<std::string>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<bool>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<bool>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (prop->isSObjectPointer)
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<SObject*>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<char>())
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<char>(*this)))
						OnPropertyChanged(*prop.get());
				}
			}
			stypeInfo = stypeInfo->GetSuper();
		}
	}
}
