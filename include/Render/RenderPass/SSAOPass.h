#pragma once
#include "Render/Export.h"
#include "Render/ScriptableRenderPass.h"
#include "Render/RenderData.h"

#include "Core/Observer.hpp"
#include "Core/GCObject.h"

namespace sh::render
{
	class RenderTexture;
	class Mesh;
	class Drawable;
	class Material;

	/// @brief depthRT 를 샘플링하여 aoRT(R8) 에 AO 값을 쓰는 풀스크린 패스.
	///        "SSAO" 태그가 RenderData 에 있을 때만 동작.
	class SSAOPass : public ScriptableRenderPass
	{
	public:
		SH_RENDER_API SSAOPass(const IRenderContext& ctx);
		SH_RENDER_API ~SSAOPass();

		SH_RENDER_API void SetMaterial(Material& mat);
	protected:
		SH_RENDER_API void Configure(const RenderData& renderData) override;
		SH_RENDER_API void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData) override;
	private:
		void EnsureDrawable();
	private:
		const IRenderContext& ctx;

		struct Resource : core::GCObject
		{
			Mesh* plane = nullptr;
			Drawable* drawable = nullptr;
			Material* mat = nullptr;

			SH_RENDER_API void PushReferenceObjects(core::GarbageCollection& gc) override;
		} resource;
		RenderData localRenderData;

		core::Observer<false, const core::SObject*>::Listener onMatDestroy;
	};
}//namespace
