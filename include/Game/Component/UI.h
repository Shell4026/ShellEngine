#pragma once
#include "../Export.h"
#include "Component.h"

#include "Core/ISyncable.h"
#include "Core/Singleton.hpp"

namespace sh::game
{
	class UI : public Component
	{
		SCLASS(UI)
	public:
		SH_GAME_API UI(GameObject& owner);

		SH_GAME_API void Update() override;
	};
}