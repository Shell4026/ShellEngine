
#include "GameObject.h"

namespace sh::game
{
	bool Component::bEditor = false;

	SH_GAME_API Component::Component(GameObject& object) :
		gameObject(object), world(object.world),
		
		bInit(false), bEnable(true)
	{
	}

	SH_GAME_API Component::Component(const Component& other) :
		SObject(other),
		gameObject(other.gameObject), world(other.world),

		bInit(other.bInit), bEnable(other.bEnable), bStart(other.bStart)
	{
	}
	SH_GAME_API Component::Component(Component&& other) noexcept :
		SObject(std::move(other)),
		gameObject(other.gameObject), world(other.world),

		bInit(other.bInit), bEnable(other.bEnable), bStart(other.bStart)
	{
		other.bEnable = false;
		other.bInit = false;
		other.bStart = false;
	}

	SH_GAME_API auto Component::operator=(const Component& other) -> Component&
	{
		bInit = other.bInit;
		bEnable = other.bEnable;

		return *this;
	}

	SH_GAME_API auto Component::IsActive() const -> bool
	{
		return bEnable;
	}

	SH_GAME_API void Component::SetActive(bool b)
	{
		bEnable = b;
		if (bEnable)
		{
			if (world.IsStart())
			{
				if (world.IsPlaying() || canPlayInEditor)
					OnEnable();
			}
		}
	}

	SH_GAME_API auto Component::IsInit() const -> bool
	{
		return bInit;
	}

	SH_GAME_API auto Component::IsStart() const -> bool
	{
		return bStart;
	}

	SH_GAME_API void Component::OnDestroy()
	{
		Super::OnDestroy();
	}

	SH_GAME_API void Component::SetPriority(int priority)
	{
		this->priority = priority;
		gameObject.RequestSortComponents();
	}

	SH_GAME_API auto Component::GetPriority() const -> int
	{
		return priority;
	}

	SH_GAME_API auto Component::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		const core::reflection::STypeInfo* type = &GetType();

		while (type != nullptr)
		{
			core::Json* json = nullptr;
			if (!mainJson.contains(type->name.ToString()))
				mainJson[type->name.ToString()] = core::Json::object();
			json = &mainJson[type->name.ToString()];

			for (auto& prop : type->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;

				const core::reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				if (propType == core::reflection::GetType<Vec4>())
					core::SerializeProperty(*json, name, *prop->Get<Vec4>(*this));
				else if (propType == core::reflection::GetType<Vec3>())
					core::SerializeProperty(*json, name, *prop->Get<Vec3>(*this));
				else if (propType == core::reflection::GetType<Vec2>())
					core::SerializeProperty(*json, name, *prop->Get<Vec2>(*this));
				else if (prop->isContainer)
				{
					auto arrJson = core::Json::array();
					if (*prop->containerElementType == core::reflection::GetType<Vec4>())
					{
						core::Json vecJson;
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							vecJson.push_back(it.Get<Vec4>()->x);
							vecJson.push_back(it.Get<Vec4>()->y);
							vecJson.push_back(it.Get<Vec4>()->z);
							vecJson.push_back(it.Get<Vec4>()->w);
						}
						arrJson.push_back(std::move(vecJson));
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec3>())
					{
						core::Json vecJson;
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							vecJson.push_back(it.Get<Vec3>()->x);
							vecJson.push_back(it.Get<Vec3>()->y);
							vecJson.push_back(it.Get<Vec3>()->z);
						}
						arrJson.push_back(std::move(vecJson));
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec2>())
					{
						core::Json vecJson;
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							vecJson.push_back(it.Get<Vec2>()->x);
							vecJson.push_back(it.Get<Vec2>()->y);
						}
						arrJson.push_back(std::move(vecJson));
					}
					if (!arrJson.empty())
						json->operator[](name) = std::move(arrJson);
				}
			}
			type = type->GetSuper();
		}
		return mainJson;
	}
	SH_GAME_API void Component::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		const core::reflection::STypeInfo* type = &GetType();
		while (type)
		{
			if (!json.contains(type->name))
			{
				type = type->GetSuper();
				continue;
			}
			const core::Json& compJson = json[type->name.ToString()];
			for (auto& prop : type->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const core::reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();

				if (propType == core::reflection::GetType<Vec4>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<Vec4>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<Vec3>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<Vec3>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<Vec2>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<Vec2>(*this)))
						OnPropertyChanged(*prop.get());
				}
				// TODO
				//else if (prop->isContainer)
				//{
				//	auto& containerJson = compJson[name];
				//	if (*prop->containerElementType == core::reflection::GetType<Vec4>())
				//	{
				//	}
				//}
			}
			type = type->GetSuper();
		}
	}
	SH_GAME_API void Component::SetEditor(bool bEditor)
	{
		Component::bEditor = bEditor;
	}
	SH_GAME_API auto Component::IsEditor() -> bool
	{
		return bEditor;
	}
}
