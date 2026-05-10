#pragma once
#include "Export.h"
#include "RenderData.h"
#include "IBuffer.h"
#include "IRenderThrMethod.h"

#include "Core/LockFreeMPSCQueue.h"
#include "Core/ArrayView.hpp"

#include "glm/vec4.hpp"

#include <memory>
#include <map>
#include <optional>
#include <vector>
namespace sh::render
{
	class IRenderContext;

	/// @brief 카메라와 같은 렌더뷰어들을 관리하는 매니저. 스레드 안전하다.
	class RenderDataManager
	{
	public:
		SH_RENDER_API RenderDataManager();
		SH_RENDER_API ~RenderDataManager();

		SH_RENDER_API void Init(const IRenderContext& ctx);

		SH_RENDER_API void PushRenderData(const RenderData& renderData);

		SH_RENDER_API auto GetBuffer() const -> const IBuffer* { return buffer.get(); }
	protected:
		SH_RENDER_API void ClearBuffer();
		SH_RENDER_API void ClearRenderViews();
		SH_RENDER_API void UploadToGPU();
		SH_RENDER_API auto GetRenderDatas() -> core::ArrayView<RenderData>;
	public:
		struct BufferData
		{
			glm::mat4 view;
			glm::mat4 proj;
			glm::vec4 pos;
		};
	private:
		friend struct IRenderThrMethod<RenderDataManager>;
		const IRenderContext* ctx = nullptr;

		core::LockFreeMPSCQueue<Ref<const RenderData>> renderDataQueue;
		std::vector<RenderData> renderDatas;

		std::unique_ptr<IBuffer> buffer;
		std::size_t alignment = 256;
		std::size_t renderDatasSize = 0;
	};

	template<>
	struct IRenderThrMethod<RenderDataManager>
	{
		static void ClearBuffer(RenderDataManager& manager) { manager.ClearBuffer(); }
		static void ClearRenderViews(RenderDataManager& manager) { manager.ClearRenderViews(); }
		static void UploadToGPU(RenderDataManager& manager) { manager.UploadToGPU(); }
		static auto GetRenderDatas(RenderDataManager& manager) -> core::ArrayView<RenderData> { return manager.GetRenderDatas(); }
	};
}//namespace
