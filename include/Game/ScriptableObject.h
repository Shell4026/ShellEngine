#pragma once
#include "Game/Export.h"

#include "Core/SObject.h"
#include "Core/Factory.hpp"
#include "Core/Logger.h"

#include <memory>
#include <string>
#define SRPO(T)\
SCLASS(T)\
struct _SRPO_Registry_##T\
{\
	using Factory = sh::core::Factory<sh::game::ScriptableObject, sh::game::ScriptableObject*, std::string>;\
	~_SRPO_Registry_##T()\
	{\
		auto& stype = T::GetStaticType();\
		auto& factory = *Factory::GetInstance();\
		SH_INFO_FORMAT("UnRegister {}", stype.type.name);\
		factory.UnRegister(std::string{ stype.type.name });\
	}\
	static auto GetStatic() -> _SRPO_Registry_##T*\
	{\
		auto& stype = T::GetStaticType();\
		auto& factory = *Factory::GetInstance();\
		if (factory.HasKey(std::string{ stype.type.name }))\
			return nullptr;\
		factory.Register(std::string{ stype.type.name }, []() {return sh::core::SObject::Create<T>(); });\
		static _SRPO_Registry_##T registry{};\
		return &registry;\
	}\
};\
inline static _SRPO_Registry_##T* _srpoRegistry = _SRPO_Registry_##T::GetStatic();\

namespace sh::game
{
	class ScriptableObject : public core::SObject
	{
		SCLASS(ScriptableObject)
	public:
		SH_GAME_API auto Serialize() const -> core::Json override;
	public:
		using Factory = sh::core::Factory<ScriptableObject, ScriptableObject*, std::string>;
	};
}//namespace