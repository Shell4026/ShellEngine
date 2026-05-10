#pragma once
#include "../Export.h"
#include "../ScriptableRenderPass.h"
#include "../IRenderThrMethod.h"

namespace sh::render
{
	class RenderTexture;
	class IBuffer;

	class CopyPass : public ScriptableRenderPass
	{
		friend struct IRenderThrMethod<CopyPass>;
	public:
		SH_RENDER_API CopyPass();
	protected:
		SH_RENDER_API void Configure(const RenderData& renderData) override;
		SH_RENDER_API void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderData& renderData) override;

		SH_RENDER_API void EnqueCopyImagePixelToBuffer(RenderTexture& rt, int x, int y, IBuffer& buffer);
		SH_RENDER_API auto IsEmpty() const -> bool { return cpyDatas.empty(); }
	private:
		struct CopyData
		{
			RenderTexture* src;
			IBuffer* dst;
			int x;
			int y;
		};
		std::vector<CopyData> cpyDatas;
	};

	template<>
	struct IRenderThrMethod<CopyPass>
	{
		static void EnqueCopyImagePixelToBuffer(CopyPass& pass, RenderTexture& rt, int x, int y, IBuffer& buffer) { pass.EnqueCopyImagePixelToBuffer(rt, x, y, buffer); }
		static auto IsEmpty(CopyPass& pass) -> bool { return pass.IsEmpty(); }
	};
}//namespace