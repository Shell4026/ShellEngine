#include "SObject.h"
#include "GarbageCollection.h"
#include "Util.h"
#include "AssetResolver.h"
#include "Logger.h"

#include <tuple>
namespace sh::core
{
	template<>
	SH_CORE_API auto DeserializeProperty(const core::Json& json, const std::string& key, SObject*& value) -> bool
	{
		if (json.contains(key))
		{
			const std::string& uuidStr = json[key].get_ref<const std::string&>();
			value = SObject::GetSObjectUsingResolver(core::UUID{ uuidStr });
			if (value == nullptr)
				return false;
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
		uuid(UUID::Generate()), name(other.name)
	{
	}
	SH_CORE_API SObject::SObject(SObject&& other) noexcept :
		bPendingKill(other.bPendingKill),
		uuid(std::move(other.uuid)), name(std::move(other.name)),
		onDestroy(std::move(other.onDestroy))
	{
		other.bPendingKill = true;
		if (other.bMark.test_and_set(std::memory_order::memory_order_acquire))
			bMark.test_and_set(std::memory_order::memory_order_relaxed);
	}
	SH_CORE_API SObject::~SObject()
	{
		SObjectManager::GetInstance()->UnRegisterSObject(this);
	}
	SH_CORE_API auto SObject::operator=(SObject&& other) noexcept -> SObject&
	{
		uuid = std::move(other.uuid);
		name = std::move(other.name);
		onDestroy = std::move(other.onDestroy);
		bPendingKill = other.bPendingKill;

		other.SetUUID(core::UUID::Generate());

		return *this;
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

	SH_CORE_API void SObject::OnPropertyChanged(const reflection::Property& prop)
	{
	}

	SH_CORE_API void SObject::Destroy()
	{
		static GarbageCollection& gc = *GarbageCollection::GetInstance();
		if (bPendingKill)
			return;

		bPendingKill = true;
		gc.RemoveRootSet(this);

		OnDestroy();
	}

	SH_CORE_API void SObject::OnDestroy()
	{
		static GarbageCollection& gc = *GarbageCollection::GetInstance();
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
	SH_CORE_API auto SObject::SetUUID(const UUID& uuid) -> bool
	{
		SObjectManager& objManager = *SObjectManager::GetInstance();
		if (objManager.GetSObject(uuid) != nullptr)
		{
			//SH_ERROR_FORMAT("UUID({}) already exists!", uuid.ToString());
			return false;
		}

		objManager.UnRegisterSObject(this);
		this->uuid = uuid;
		RegisterToManager(this);
		return true;
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
				else if (prop->isSObjectPointerContainer)
				{
					json[name] = core::Json::array();
					for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
					{
						assert(!it.IsPair());
						if (!it.IsPair()) // pair인 경우는 map, unordered_map
						{
							auto ptr = it.Get<core::SObject*>();
							if (core::IsValid(*ptr))
								json[name].push_back((*ptr)->GetUUID().ToString());
						}
					}
				}
				else if (prop->isContainer)
				{
					json[name] = core::Json::array();
					if (*prop->containerElementType == core::reflection::GetType<int>() || *prop->containerElementType == core::reflection::GetType<uint32_t>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<int>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<int16_t>() || *prop->containerElementType == core::reflection::GetType<uint16_t>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<int16_t>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<float>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<float>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<double>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<double>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<bool>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<bool>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<char>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<char>());
					}
					else if (*prop->containerElementType == core::reflection::GetType<std::string>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
							json[name].push_back(*it.Get<std::string>());
					}
				}
			}
			if (!json.empty())
				mainJson[stypeInfo->name] = std::move(json);
			stypeInfo = stypeInfo->super;
		}
		return mainJson;
	}
	SH_CORE_API void SObject::Deserialize(const Json& json)
	{
		const reflection::STypeInfo* stypeInfo = &GetType();
		if (stypeInfo->name != json["type"].get_ref<const std::string&>())
			return;

		if (json.contains("uuid"))
		{
			UUID newUUID{ json["uuid"].get_ref<const std::string&>() };
			if (uuid != newUUID)
			{
				auto objPtr = core::SObjectManager::GetInstance()->GetSObject(newUUID);
				if (objPtr != nullptr && objPtr->IsPendingKill())
					objPtr->SetUUID(core::UUID::Generate());
				bool success = SetUUID(std::move(newUUID));
				assert(success);
			}
		}
		if (json.contains("name"))
			SetName(json["name"].get_ref<const std::string&>());

		while (stypeInfo)
		{
			if (!json.contains(stypeInfo->name))
			{
				stypeInfo = stypeInfo->super;
				continue;
			}
			const core::Json& subJson = json[stypeInfo->name];
			for (auto& prop : stypeInfo->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				// 해당 타입들에 대해 if문으로 검사 후 역직렬화
				// 코드에서 똥 냄새 난다
				using Types = std::tuple<int, uint32_t, int64_t, uint64_t, int16_t, uint16_t, float, double, bool, char, std::string>;
				bool bDone = false;
				std::apply(
					[&](auto&&... args) 
					{
						(
							[&](auto t)
							{
								using Type = std::decay_t<decltype(t)>;
								if (!bDone && propType == core::reflection::GetType<Type>())
								{
									if (core::DeserializeProperty(subJson, name, *prop->Get<Type>(*this)))
										OnPropertyChanged(*prop.get());
									bDone = true;
								}
							}(std::forward<decltype(args)>(args)), ...
						);
					}, Types{}
				);
				if (bDone)
					continue;

				if (prop->isEnum)
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<int>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (prop->isSObjectPointer)
				{
					if (core::DeserializeProperty(subJson, name, *prop->Get<SObject*>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (prop->isSObjectPointerContainer)
				{
					if (subJson.contains(name) && subJson[name].is_array())
					{
						prop->ClearContainer(*this);
						for (auto& uuidStr : subJson[name])
							prop->InsertToContainer(*this, GetSObjectUsingResolver(core::UUID{ uuidStr.get_ref<const std::string&>() }));
						OnPropertyChanged(*prop.get());
					}
				}
				else if (prop->isContainer)
				{
					if (!subJson.contains(name) || !subJson[name].is_array())
						continue;
					prop->ClearContainer(*this);
					
					// c++20이면 람다식에도 템플릿이 된다던데..
					auto insertFn =
						[&](auto type)
						{
							for (auto& arr : subJson[name])
								prop->InsertToContainer(*this, arr.get<decltype(type)>());
						}; 
					if (*prop->containerElementType == core::reflection::GetType<int>() || *prop->containerElementType == core::reflection::GetType<uint32_t>())
						insertFn(0);
					else if (*prop->containerElementType == core::reflection::GetType<int16_t>() || *prop->containerElementType == core::reflection::GetType<uint16_t>())
						insertFn((int16_t)0);
					else if (*prop->containerElementType == core::reflection::GetType<float>())
						insertFn(0.f);
					else if (*prop->containerElementType == core::reflection::GetType<double>())
						insertFn(0.0);
					else if (*prop->containerElementType == core::reflection::GetType<bool>())
						insertFn(false);
					else if (*prop->containerElementType == core::reflection::GetType<char>())
						insertFn('\0');
					else if (*prop->containerElementType == core::reflection::GetType<std::string>())
						insertFn(std::string{});
					OnPropertyChanged(*prop.get());
				}
			}//for
			stypeInfo = stypeInfo->super;
		}
	}
	SH_CORE_API auto SObject::GetSObjectUsingResolver(const core::UUID& uuid) -> core::SObject*
	{
		static SObjectManager* objectManager = SObjectManager::GetInstance();

		SObject* objPtr = objectManager->GetSObject(uuid);
		if (objPtr == nullptr)
		{
			static auto& resolver = AssetResolverRegistry::GetResolver();
			if (resolver)
				objPtr = resolver(uuid);
			else
				return nullptr;
		}
		return objPtr;
	}
}//namespace