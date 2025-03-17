#pragma once
#include "Export.h"
#include "ShaderPass.h"
#include "Drawable.h"
#include "Camera.h"

#include <vector>
#include <memory>
namespace sh::render
{
	class RenderPipeline
	{
	private:
		std::vector<std::unique_ptr<ShaderPass>> opaquePass;
		std::vector<std::unique_ptr<ShaderPass>> lightingPass;
	public:
		SH_RENDER_API virtual void Render(const Drawable& drawable, const Camera* camera = nullptr);
	};
}//namespace