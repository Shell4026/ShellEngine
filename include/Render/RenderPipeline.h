#pragma once
#include "Export.h"
#include "Drawable.h"

#include "core/Name.h"
#include "Core/Observer.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <memory>
namespace sh::render
{
	class IRenderContext;
	class Camera;

	struct RenderGroup
	{
		const Material* material;
		Mesh::Topology topology;
		std::vector<Drawable*> drawables;
	};

	class IRenderPipelineImpl
	{
	public:
		virtual ~IRenderPipelineImpl() = default;

		virtual void RecordCommand(const core::Name& lightingPassName, const std::vector<const Camera*>& cameras, const std::vector<RenderGroup>& renderData, uint32_t imgIdx) = 0;
		
		/// @brief 렌더링 시 이전에 그려진 프레임 버퍼를 지울지 설정
		/// @param bClear 지운다면 true 아니면 false
		virtual void SetClear(bool bClear) = 0;
		virtual auto GetDrawCallCount() const -> uint32_t = 0;
	};

	class RenderPipeline
	{
	private:
		const Material* replacementMat = nullptr;

		core::Observer<false, const core::SObject*>::Listener materialDestroyListener;
		
		bool bClear = true;
	protected:
		std::vector<RenderGroup> renderGroups;
		std::vector<const Camera*> ignoreCameras;

		core::Name passName = core::Name{ "Forward" };

		std::unique_ptr<IRenderPipelineImpl> impl;
	public:
		SH_RENDER_API RenderPipeline();
		SH_RENDER_API virtual ~RenderPipeline();

		SH_RENDER_API virtual void Create(IRenderContext& context);

		SH_RENDER_API virtual void PushDrawable(Drawable* drawable);
		SH_RENDER_API virtual void ClearDrawable();

		SH_RENDER_API virtual void RecordCommand(const std::vector<const Camera*>& cameras, uint32_t imgIdx);

		/// @brief 렌더링 될 때 모든 객체를 해당 메테리얼로 렌더링 한다.
		/// @param mat 메테리얼 포인터
		SH_RENDER_API void SetReplacementMaterial(const Material* mat);
		/// @brief 렌더링 시 이전에 그려진 프레임 버퍼를 지울지 설정
		/// @param bClear 지운다면 true 아니면 false
		SH_RENDER_API void SetClear(bool bClear);

		SH_RENDER_API void IgnoreCamera(const Camera& camera);

		SH_RENDER_API auto GetDrawCallCount() const -> uint32_t;
		SH_RENDER_API auto GetPassName() const -> const core::Name&;

		SH_RENDER_API auto GetImpl() const -> IRenderPipelineImpl*;
	};
}//namespace