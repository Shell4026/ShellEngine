#include "ScriptableObject.h"

namespace sh::game
{
	SH_GAME_API auto ScriptableObject::Serialize() const -> core::Json
	{
		core::Json json = Super::Serialize();
		json["fullType"] = GetType().type.name;
		return json;
	}
}//namespace