#include "RenderDataManager.h"
#include "BufferFactory.h"

#include "Core/Util.h"
namespace sh::render
{
	RenderDataManager::RenderDataManager() = default;
	RenderDataManager::~RenderDataManager() = default;

	SH_RENDER_API void RenderDataManager::Init(const IRenderContext& ctx)
	{
		if (this->ctx != nullptr)
			return;

		this->ctx = &ctx;
		alignment = core::Util::AlignTo(sizeof(BufferData), BufferFactory::GetBufferAlignment(ctx));;

		const std::size_t cap = 100;

		BufferFactory::CreateInfo info{};
		info.size = cap * alignment;
		info.bDynamic = false;
		info.bGPUOnly = false;

		buffer = BufferFactory::Create(ctx, info);
	}
	SH_RENDER_API void RenderDataManager::PushRenderData(const RenderData& renderTarget)
	{
		renderDataQueue.Push(renderTarget);
	}
	SH_RENDER_API void RenderDataManager::ClearBuffer()
	{
		buffer.reset();
	}
	SH_RENDER_API void RenderDataManager::ClearRenderViews()
	{
		renderDataQueue.Clear();
		renderDatas.clear();
	}
	SH_RENDER_API void RenderDataManager::UploadToGPU()
	{
		std::size_t offset = 0;
		std::size_t i = 0;
		renderDatasSize = 0;
		renderDataQueue.Drain(
			[&](Ref<const RenderData>& renderTarget)
			{
				if (offset + sizeof(BufferData) > buffer->GetSize())
					return;

				if (renderDatas.size() <= i)  // 메모) 힙 할당 최소화 하기 위해 이렇게
					renderDatas.resize(i + 1);

				renderDatas[i] = renderTarget.get();

				for (RenderViewer& viewer : renderDatas[i].renderViewers)
				{
					BufferData data;
					data.view = viewer.viewMatrix;
					data.proj = viewer.projMatrix;
					data.pos = viewer.pos;

					buffer->SetData(&data, offset, sizeof(BufferData));

					viewer.offset = offset;
					offset += alignment;
				}
				++i;
			}
		);
		renderDatasSize = i;
		std::sort(renderDatas.begin(), renderDatas.begin() + renderDatasSize,
			[](const RenderData& left, const RenderData& right)
			{
				return left.priority > right.priority;
			}
		);
	}

	SH_RENDER_API auto RenderDataManager::GetRenderDatas() -> core::ArrayView<RenderData>
	{
		return core::ArrayView<RenderData>{renderDatas.data(), renderDatasSize};
	}
}//namespace