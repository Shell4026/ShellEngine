#pragma once
#include "../Export.h"
#include "../ScriptableRenderPass.h"
#include "../IRenderThrMethod.h"

namespace sh::render
{
	class RenderTexture;
	class IBuffer;

	template<>
	struct IRenderThrMethod<class CopyPass>
	{
		static void EnqueCopyImagePixelToBuffer(CopyPass& pass, RenderTexture& rt, int x, int y, IBuffer& buffer);
	};

	class CopyPass : public ScriptableRenderPass
	{
		friend IRenderThrMethod<class CopyPass>;
	public:
		SH_RENDER_API CopyPass();
	protected:
		SH_RENDER_API void Configure(const RenderTarget& renderData) override;
		SH_RENDER_API void Record(CommandBuffer& cmd, const IRenderContext& ctx, const RenderTarget& renderData) override;
		SH_RENDER_API auto BuildDrawList(const RenderTarget& renderData) -> DrawList override;

		SH_RENDER_API void EnqueCopyImagePixelToBuffer(RenderTexture& rt, int x, int y, IBuffer& buffer);
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

	inline void IRenderThrMethod<class CopyPass>::EnqueCopyImagePixelToBuffer(CopyPass& pass, RenderTexture& rt, int x, int y, IBuffer& buffer)
	{
		pass.EnqueCopyImagePixelToBuffer(rt, x, y, buffer);
	}
}//namespace