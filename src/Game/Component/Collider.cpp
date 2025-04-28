#include "Component/Collider.h"

namespace sh::game
{
	Collider::Collider(GameObject& owner) :
		Component(owner)
	{
		canPlayInEditor = true;
	}
}//namespace