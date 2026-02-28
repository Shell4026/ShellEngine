#include "Component/Component.h"
#include "World.h"

namespace sh::game
{
	bool Component::bEditor = false;

	SH_GAME_API Component::Component(GameObject& object) :
		gameObject(object), world(object.world)
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

	SH_GAME_API auto Component::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		const core::reflection::STypeInfo* type = &GetType();

		while (type != nullptr)
		{
			core::Json& json = mainJson[type->name.ToString()];

			for (auto& prop : type->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;

				const core::reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();
				const std::string& nameStr = name.ToString();

				if (propType == core::reflection::GetType<Vec4>())
					core::SerializeProperty(json, nameStr, *prop->Get<Vec4>(*this));
				else if (propType == core::reflection::GetType<Vec3>())
					core::SerializeProperty(json, nameStr, *prop->Get<Vec3>(*this));
				else if (propType == core::reflection::GetType<Vec2>())
					core::SerializeProperty(json, nameStr, *prop->Get<Vec2>(*this));
				else if (prop->isContainer)
				{
					auto& containerJson = json[nameStr];
					if (*prop->containerElementType == core::reflection::GetType<Vec4>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							core::Json vecJson;
							vecJson.push_back(it.Get<Vec4>()->x);
							vecJson.push_back(it.Get<Vec4>()->y);
							vecJson.push_back(it.Get<Vec4>()->z);
							vecJson.push_back(it.Get<Vec4>()->w);

							containerJson.push_back(std::move(vecJson));
						}
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec3>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							core::Json vecJson;
							vecJson.push_back(it.Get<Vec3>()->x);
							vecJson.push_back(it.Get<Vec3>()->y);
							vecJson.push_back(it.Get<Vec3>()->z);

							containerJson.push_back(std::move(vecJson));
						}
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec2>())
					{
						for (auto it = prop->Begin(*this); it != prop->End(*this); ++it)
						{
							core::Json vecJson;
							vecJson.push_back(it.Get<Vec2>()->x);
							vecJson.push_back(it.Get<Vec2>()->y);

							containerJson.push_back(std::move(vecJson));
						}
					}
				}
			}
			type = type->super;
		}

		mainJson["fullType"] = GetType().type.name;
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
				type = type->super;
				continue;
			}
			const core::Json& compJson = json[type->name.ToString()];
			for (auto& prop : type->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const core::reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();
				const std::string& nameStr = name.ToString();

				if (propType == core::reflection::GetType<Vec4>() && compJson[nameStr].size() == 4)
				{
					if (core::DeserializeProperty(compJson, nameStr, *prop->Get<Vec4>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<Vec3>() && compJson[nameStr].size() == 3)
				{
					if (core::DeserializeProperty(compJson, nameStr, *prop->Get<Vec3>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<Vec2>() && compJson[nameStr].size() == 2)
				{
					if (core::DeserializeProperty(compJson, nameStr, *prop->Get<Vec2>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (prop->isContainer)
				{
					const auto& containerJson = compJson[nameStr];
					if (*prop->containerElementType == core::reflection::GetType<Vec4>())
					{
						prop->ClearContainer(*this);
						for (auto& vecJson : containerJson)
						{
							if (vecJson.size() == 4)
								prop->InsertToContainer(*this, Vec4{ vecJson[0], vecJson[1], vecJson[2], vecJson[3] });
						}
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec3>())
					{
						prop->ClearContainer(*this);
						for (auto& vecJson : containerJson)
						{
							if (vecJson.size() == 3)
								prop->InsertToContainer(*this, Vec3{ vecJson[0], vecJson[1], vecJson[2] });
						}
					}
					else if (*prop->containerElementType == core::reflection::GetType<Vec2>())
					{
						prop->ClearContainer(*this);
						for (auto& vecJson : containerJson)
						{
							if (vecJson.size() == 2)
								prop->InsertToContainer(*this, Vec3{ vecJson[0], vecJson[1] });
						}
					}
				}
			}
			type = type->super;
		}
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

	SH_GAME_API void Component::SetPriority(int priority)
	{
		this->priority = priority;
		gameObject.RequestSortComponents();
	}

	SH_GAME_API void Component::SetIsEditor(bool bEditor)
	{
		Component::bEditor = bEditor;
	}
}//namespace