#pragma once
#include "../Export.h"
#include "../ScriptableRenderPass.h"

#include <glm/mat4x4.hpp>

#include <vector>
namespace sh::render
{
	class Camera;
	class RenderTexture;

	class ShadowMapPass : public ScriptableRenderPass
	{
	public:
		SH_RENDER_API ShadowMapPass();
	};
}//namespace
