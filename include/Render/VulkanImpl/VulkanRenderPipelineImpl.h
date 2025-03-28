#pragma once
#include "Export.h"
#include "Render/RenderPipeline.h"
#include "Render/Mesh.h"

#include <vector>
#include <string>
#include <cstdint>

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanCommandBuffer;
	class VulkanCameraBuffers;
	class VulkanShaderPass;

	class VulkanRenderPipelineImpl : public IRenderPipelineImpl
	{
	private:
		VulkanContext& context;
		VulkanCommandBuffer* cmd = nullptr;

		VulkanCameraBuffers* cameraManager = nullptr;

		uint32_t drawCall = 0;
	protected:
		SH_RENDER_API virtual void RenderDrawable(const core::Name& lightingPassName, const Camera& camera, const std::vector<RenderGroup>& renderGroups, VkRenderPass renderPass);
	public:
		SH_RENDER_API VulkanRenderPipelineImpl(VulkanContext& context);

		SH_RENDER_API void RecordCommand(const core::Name& lightingPassName, const std::vector<const Camera*>& cameras, const std::vector<RenderGroup>& renderData, uint32_t imgIdx) override;

		SH_RENDER_API auto GetDrawCallCount() const -> uint32_t override;
	};
}//namespace