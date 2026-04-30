#include "RenderPass/ShadowMapPass.h"
#include "RenderTexture.h"
#include "Camera.h"
#include "Drawable.h"
#include "Material.h"
#include "Shader.h"
#include "IRenderContext.h"

namespace sh::render
{
	ShadowMapPass::ShadowMapPass() :
		ScriptableRenderPass(core::Name{ "ShadowMapPass" }, RenderQueue::BeforeRendering)
	{
	}
}//namespace
