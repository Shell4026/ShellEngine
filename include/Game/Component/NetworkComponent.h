#pragma once
#include "Export.h"
#include "Component.h"

namespace sh::game
{
	class NetworkComponent : public Component
	{
		SCLASS(NetworkComponent)
	public:
		SH_GAME_API NetworkComponent(GameObject& owner);
	};
}//namespace