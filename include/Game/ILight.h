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
		virtual auto GetLightType() const -> Type = 0;

		virtual auto IsCastShadow() const -> bool { return false; }
		virtual void SetCastShadow(bool b) {}
		virtual auto GetShadowBias() const -> float { return 0.005f; }
		virtual void SetShadowBias(float bias) {}
		virtual auto GetShadowMapResolution() const -> uint32_t { return 1024; }
		virtual void SetShadowMapResolution(uint32_t res) {}
	};
}//namespace
