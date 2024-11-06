#pragma once

#include "Export.h"
#include "Component/Component.h"
#include "IOctreeElement.h"

namespace sh::render
{
	class Material;
}
namespace sh::game
{
	class ILight : public Component, public IOctreeElement
	{
		SCLASS(ILight)
	public:
		ILight(GameObject& owner);
		virtual ~ILight() = default;
		virtual void SetIntensity(float intensity) = 0;
		//virtual void SetMaterialData(render::Material& mat) = 0;
	};
}//namespace