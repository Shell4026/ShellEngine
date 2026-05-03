#include "RenderPass/ShadowMapPass.h"
#include "IRenderContext.h"

namespace sh::render
{
	ShadowMapPass::ShadowMapPass() :
		ScriptableRenderPass(core::Name{ "ShadowMapPass" }, RenderQueue::BeforeRendering)
	{
	}
}//namespace
