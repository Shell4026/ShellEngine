#pragma once
#include "Export.h"
#include "IOctreeElement.h"

namespace sh::render
{
	class Material;
}
namespace sh::game
{
	class ILight : public IOctreeElement
	{
	public:
		enum class Type
		{
			Point,
			Directional
		};
	public:
		virtual ~ILight() {};
		virtual void SetIntensity(float intensity) = 0;
		virtual auto GetLightType() const -> Type = 0;
		//virtual void SetMaterialData(render::Material& mat) = 0;
	};
}//namespace