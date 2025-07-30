#include "VulkanCameraBuffers.h"

#include "Core/Util.h"

namespace sh::render::vk
{
	void VulkanCameraBuffers::Init(const VulkanContext& context)
	{
		this->context = &context;

		cameraData = std::make_unique<VulkanBuffer>(context);
		std::size_t minUboAlignment = context.GetGPUProperty().limits.minUniformBufferOffsetAlignment;

		dynamicAlignment = core::Util::AlignTo(sizeof(Camera::BufferData), minUboAlignment);

		std::size_t bufferSize = cap * dynamicAlignment;

		cameraData->Create(bufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			true);
	}
	SH_RENDER_API void VulkanCameraBuffers::Clear()
	{
		cams.clear();
		emptyidx = std::stack<uint32_t>{};
	}
	void VulkanCameraBuffers::AddCamera(const Camera& camera)
	{
		assert(cams.size() < cap);
		if (emptyidx.empty())
		{
			cams.push_back(&camera);
		}
		else
		{
			uint32_t idx = emptyidx.top();
			emptyidx.pop();
			cams[idx] = &camera;
		}
	}
	void VulkanCameraBuffers::RemoveCamera(const Camera& camera)
	{
		auto it = std::find(cams.begin(), cams.end(), &camera);
		if (it == cams.end())
			return;
		*it = nullptr;
		emptyidx.push(std::distance(cams.begin(), it));
	}
	void VulkanCameraBuffers::UploadDataToGPU(const Camera& camera)
	{
		auto it = std::find(cams.begin(), cams.end(), &camera);
		if (it == cams.end())
			return;
		std::size_t offset = GetDynamicOffset(camera);
		Camera::BufferData* data = reinterpret_cast<Camera::BufferData*>((std::size_t)cameraData->GetData() + offset);
		data->matView = camera.GetViewMatrix(core::ThreadType::Render);
		data->matProj = camera.GetProjMatrix(core::ThreadType::Render);
	}
	auto VulkanCameraBuffers::GetDynamicOffset(const Camera& camera) const -> uint32_t
	{
		auto it = std::find(cams.begin(), cams.end(), &camera);
		if (it == cams.end())
			return 0;
		uint32_t idx = std::distance(cams.begin(), it);
		return idx * dynamicAlignment;
	}
	SH_RENDER_API auto sh::render::vk::VulkanCameraBuffers::GetCameraBuffer() const -> const VulkanBuffer&
	{
		return *cameraData.get();
	}
}//namespace