#pragma once
#include "Render/Export.h"
#include "Render/Camera.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

#include "Core/Singleton.hpp"

#include <vector>
#include <cstdint>
#include <stack>
#include <memory>
namespace sh::render::vk
{
	class VulkanCameraBuffers : public core::Singleton<VulkanCameraBuffers>
	{
		friend core::Singleton<VulkanCameraBuffers>;
	private:
		const VulkanContext* context = nullptr;

		std::vector<const Camera*> cams;
		uint32_t cap = 10;
		std::stack<uint32_t> emptyidx;

		std::unique_ptr<VulkanBuffer> cameraData;

		std::size_t dynamicAlignment;
	private:
		VulkanCameraBuffers() = default;
	public:
		SH_RENDER_API void Init(const VulkanContext& context);
		SH_RENDER_API void Clear();

		SH_RENDER_API void AddCamera(const Camera& camera);
		SH_RENDER_API void RemoveCamera(const Camera& camera);
		SH_RENDER_API void UploadDataToGPU(const Camera& camera);

		SH_RENDER_API auto GetDynamicOffset(const Camera& camera) const -> uint32_t;
		SH_RENDER_API auto GetCameraBuffer() const -> const VulkanBuffer&;
	};
}//namespace