#pragma once
#include "Drawable.h"

#include <string>
#include <string_view>

namespace sh::render
{
	class IRenderContext;
	class Camera;

	class ILightingPass
	{
	public:
		virtual ~ILightingPass() = default;

		virtual void Init(IRenderContext& context) = 0;
		virtual void PushDrawable(Drawable* drawable) = 0;
		virtual void ClearDrawable() = 0;
		virtual void RecordCommand(const Camera& camera, uint32_t imgIdx) = 0;
		virtual auto GetName() const -> const std::string& = 0;
	};
}//namespace