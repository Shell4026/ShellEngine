#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

namespace sh::game
{
	class NetworkComponent : public Component
	{
		SCLASS(NetworkComponent)
	public:
		SH_GAME_API NetworkComponent(GameObject& owner);
	};
}//namespace