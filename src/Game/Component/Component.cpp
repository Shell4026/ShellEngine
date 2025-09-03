
#include "GameObject.h"

namespace sh::game
{
	SH_GAME_API Component::Component(GameObject& object) :
		gameObject(object), world(object.world),
		
		bInit(false), bEnable(true)
	{
	}

	SH_GAME_API Component::Component(const Component& other) :
		SObject(other),
		gameObject(other.gameObject), world(other.world),

		bInit(other.bInit), bEnable(other.bEnable)
	{
	}
	SH_GAME_API Component::Component(Component&& other) noexcept :
		SObject(std::move(other)),
		gameObject(other.gameObject), world(other.world),

		bInit(other.bInit), bEnable(other.bEnable)
	{
		other.bEnable = false;
		other.bInit = false;
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
			if (!bInit)
			{
				if (world.IsPlaying() || canPlayInEditor)
				{
					Awake();
					bInit = true;
				}
			}
			if (world.IsPlaying() || canPlayInEditor)
				OnEnable();
		}
	}

	SH_GAME_API auto Component::IsInit() const -> bool
	{
		return bInit;
	}

	SH_GAME_API void Component::Awake()
	{
		bInit = true;
	}
	SH_GAME_API void Component::Start()
	{

	}
	SH_GAME_API void Component::OnEnable()
	{

	}
	SH_GAME_API void Component::FixedUpdate()
	{
	}
	SH_GAME_API void Component::BeginUpdate()
	{
	}
	SH_GAME_API void Component::Update()
	{
	}
	SH_GAME_API void Component::LateUpdate()
	{
	}
	
	SH_GAME_API void Component::OnDestroy()
	{
		Super::OnDestroy();
	}

	SH_GAME_API auto Component::Serialize() const -> core::Json
	{
		core::Json mainJson{ Super::Serialize() };
		const core::reflection::STypeInfo* type = &GetType();
		while (type)
		{
			core::Json json{};
			for (auto& prop :type->GetProperties())
			{
				if (prop->bNoSaveProperty)
					continue;
				const core::reflection::TypeInfo& propType = prop->type;
				const core::Name& name = prop->GetName();
				if (propType == core::reflection::GetType<int>() || prop->isEnum)
					core::SerializeProperty(json, name, *prop->Get<int>(*this));
				else if (propType == core::reflection::GetType<float>())
					core::SerializeProperty(json, name, *prop->Get<float>(*this));
				else if (propType == core::reflection::GetType<std::string>())
					core::SerializeProperty(json, name, *prop->Get<std::string>(*this));
				else if (propType == core::reflection::GetType<bool>())
					core::SerializeProperty(json, name, *prop->Get<bool>(*this));
				else if (propType == core::reflection::GetType<Vec4>())
					core::SerializeProperty(json, name, *prop->Get<Vec4>(*this));
				else if (propType == core::reflection::GetType<Vec3>())
					core::SerializeProperty(json, name, *prop->Get<Vec3>(*this));
				else if (propType == core::reflection::GetType<Vec2>())
					core::SerializeProperty(json, name, *prop->Get<Vec2>(*this));
				else if (prop->isSObjectPointer)
					core::SerializeProperty(json, name, *prop->Get<SObject*>(*this));
			}
			if (!json.empty())
				mainJson[type->name] = json;
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

				if (propType == core::reflection::GetType<int>() || prop->isEnum)
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<int>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<float>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<float>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<std::string>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<std::string>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<bool>())
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<bool>(*this)))
						OnPropertyChanged(*prop.get());
				}
				else if (propType == core::reflection::GetType<Vec4>())
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
				else if (prop->isSObjectPointer)
				{
					if (core::DeserializeProperty(compJson, name, *prop->Get<SObject*>(*this)))
						OnPropertyChanged(*prop.get());
				}
			}
			type = type->GetSuper();
		}
	}
}
