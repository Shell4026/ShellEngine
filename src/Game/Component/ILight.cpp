#include "Component/ILight.h"

namespace sh::game
{
	ILight::ILight(GameObject& owner) :
		Component(owner)
	{
	}
}//namespace