#include "RenderPipeline.h"

#include "VulkanImpl/VulkanContext.h"
#include "VulkanImpl/VulkanRenderPipelineImpl.h"

#include "Core/GarbageCollection.h"

namespace sh::render
{
	RenderPipeline::RenderPipeline()
	{
		materialDestroyListener.SetCallback([&](const core::SObject* obj)
			{
				replacementMat = nullptr;
			}
		);
	}
	RenderPipeline::~RenderPipeline()
	{
	}

	SH_RENDER_API void RenderPipeline::Create(IRenderContext& context)
	{
		assert(context.GetRenderAPIType() == render::RenderAPI::Vulkan); // 나중에 API 추가 할 일 있으면 지우기
		if (context.GetRenderAPIType() == render::RenderAPI::Vulkan)
		{
			impl = std::make_unique<vk::VulkanRenderPipelineImpl>(static_cast<vk::VulkanContext&>(context));
			impl->SetClear(bClear);
		}
	}

	SH_RENDER_API void RenderPipeline::PushDrawable(Drawable* drawable)
	{
		if (!drawable->CheckAssetValid())
			return;
		if (drawable->GetMaterial()->GetShader()->GetShaderPasses(GetPassName()) == nullptr)
			return;

		const Material* mat = drawable->GetMaterial();
		if (replacementMat)
			mat = replacementMat;
		Mesh::Topology topology = drawable->GetMesh()->GetTopology();

		RenderGroup* renderGroup = nullptr;
		for (auto& group : renderGroups)
		{
			if (group.material == mat && group.topology == topology)
			{
				renderGroup = &group;
				break;
			}
		}
		if (renderGroup == nullptr)
		{
			RenderGroup group{};
			group.material = mat;
			group.topology = topology;
			group.drawables.push_back(drawable);
			renderGroups.push_back(std::move(group));
			
			core::GarbageCollection::GetInstance()->AddVectorTracking(renderGroups.back().drawables);
		}
		else
		{
			renderGroup->drawables.push_back(drawable);
		}
	}
	SH_RENDER_API void RenderPipeline::ClearDrawable()
	{
		for (auto& renderGroup : renderGroups)
			core::GarbageCollection::GetInstance()->RemoveVectorTracking(renderGroups.back().drawables);

		renderGroups.clear();
	}

	SH_RENDER_API void RenderPipeline::RecordCommand(const std::vector<const Camera*>& cameras, uint32_t imgIdx)
	{
		assert(impl);
		std::vector<const Camera*> camVector;
		camVector.reserve(cameras.size());
		for (auto cam : cameras)
		{
			if (auto it = std::find(ignoreCameras.begin(), ignoreCameras.end(), cam); it == ignoreCameras.end())
				camVector.push_back(cam);
		}
		impl->RecordCommand(passName, camVector, renderGroups, imgIdx);
	}
	SH_RENDER_API void RenderPipeline::SetReplacementMaterial(const Material* mat)
	{
		if (replacementMat != nullptr)
			replacementMat->onDestroy.UnRegister(materialDestroyListener);

		if (mat == nullptr)
		{
			replacementMat = nullptr;
		}
		else
		{
			mat->onDestroy.Register(materialDestroyListener);
			replacementMat = mat;
		}
	}
	SH_RENDER_API void RenderPipeline::SetClear(bool bClear)
	{
		this->bClear = bClear;
		if (impl)
			impl->SetClear(bClear);
	}
	SH_RENDER_API void RenderPipeline::IgnoreCamera(const Camera& camera)
	{
		auto it = std::find(ignoreCameras.begin(), ignoreCameras.end(), &camera);
		if (it == ignoreCameras.end())
			ignoreCameras.push_back(&camera);
	}
	SH_RENDER_API auto RenderPipeline::GetDrawCallCount() const -> uint32_t
	{
		assert(impl);
		return impl->GetDrawCallCount();
	}
	SH_RENDER_API auto RenderPipeline::GetPassName() const -> const core::Name&
	{
		return passName;
	}
	SH_RENDER_API auto RenderPipeline::GetImpl() const -> IRenderPipelineImpl*
	{
		return impl.get();
	}
}//namespace