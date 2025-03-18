#pragma once
#include "Export.h"
#include "Render/ILightingPass.h"
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

	class VulkanBasePass : public ILightingPass
	{
	private:
		struct RenderGroup
		{
			const Material* material;
			Mesh::Topology topology;
			std::vector<Drawable*> drawables;
		};
		VulkanContext* context = nullptr;
		VulkanCommandBuffer* cmd = nullptr;
		std::string passName = "Forward";

		std::vector<RenderGroup> renderGroups;

		VulkanCameraBuffers* cameraManager;
	protected:
		SH_RENDER_API virtual void RenderDrawable(const Camera& camera, VkRenderPass renderPass);
	public:
		SH_RENDER_API void Init(IRenderContext& context) override;
		SH_RENDER_API void PushDrawable(Drawable* drawable) override;
		SH_RENDER_API void ClearDrawable() override;
		SH_RENDER_API void RecordCommand(const Camera& camera, uint32_t imgIdx) override;

		SH_RENDER_API auto GetName() const -> const std::string& override;
	};
}//namespace