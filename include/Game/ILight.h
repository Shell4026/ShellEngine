#pragma once
#include "Game/Export.h"
#include "IOctreeElement.h"

#include <cstdint>

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
		virtual auto GetIntensity() const -> float = 0;
		virtual auto GetLightType() const -> Type = 0;
		virtual auto GetLightSpaceMatrix() const -> glm::mat4 = 0;
	};
}//namespace
