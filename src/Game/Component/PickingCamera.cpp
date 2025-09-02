#include "Component/PickingCamera.h"

#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/RenderThread.h"

#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanFramebuffer.h"
#include "Render/VulkanImpl/VulkanImageBuffer.h"
#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/VulkanImpl/VulkanBuffer.h"
#include "Render/BufferFactory.h"

namespace sh::game
{
	PickingCamera::PixelData::operator uint32_t() const
	{
		uint32_t id = r | g << 8 | b << 24;
		return id;
	}

	PickingCamera::PickingCamera(GameObject& owner) :
		Camera(owner)
	{
		renderTex = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::RGBA32, false);
		renderTex->SetReadUsage(true);
		renderTex->SetSize(1024, 768);
		renderTex->Build(*world.renderer.GetContext());
		camera.SetWidth(1024);
		camera.SetHeight(768);
		renderTex->SetName("PickingFramebuffer");

		SetRenderTexture(renderTex);

		buffer = render::BufferFactory::Create(*world.renderer.GetContext(), sizeof(uint8_t) * 4, true);
	}

	SH_GAME_API void PickingCamera::Awake()
	{
		Super::Awake();
	}

	SH_GAME_API void PickingCamera::BeginUpdate()
	{
		//SH_INFO_FORMAT("{}, {}, {}", gameObject.transform->GetWorldPosition().x, gameObject.transform->GetWorldPosition().y, gameObject.transform->GetWorldPosition().z);
		if (core::IsValid(followCamera))
		{
			SetLookPos(followCamera->GetLookPos());
			SetUpVector(followCamera->GetUpVector());
		}
		Super::BeginUpdate();
		if (!pickingCallback.Empty())
		{
			assert(world.renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan);
			if (world.renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan)
			{
				auto& vkRenderer = static_cast<render::vk::VulkanRenderer&>(world.renderer);
				auto vkFramebuffer = static_cast<render::vk::VulkanFramebuffer*>(renderTex->GetFramebuffer());
				auto& vkContext = static_cast<render::vk::VulkanContext&>(*world.renderer.GetContext());
				auto vkBuffer = static_cast<render::vk::VulkanBuffer*>(buffer.get());

				VkSemaphore timelineSemaphore = vkRenderer.GetTimelineSemaphore();
				// 첫번째 패스의 타임라인 값을 가져오는데, 첫번째 패스는 pickingPass다.
				// 즉, 피킹 패스가 끝날 때 까지 대기하는 코드.
				uint64_t timelineValue = vkRenderer.GetTimelineValue();

				VkSemaphoreWaitInfo info{};
				info.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
				info.semaphoreCount = 1;
				info.pSemaphores = &timelineSemaphore;
				info.pValues = &timelineValue;
				info.flags = VkSemaphoreWaitFlagBits::VK_SEMAPHORE_WAIT_ANY_BIT;

				auto result = vkWaitSemaphores(vkContext.GetDevice(), &info, std::numeric_limits<uint64_t>::max());
				assert(result == VkResult::VK_SUCCESS);
				vkFramebuffer->TransferImageToBuffer(vkBuffer->GetBuffer(), x, y);
				pixels = reinterpret_cast<uint8_t*>(buffer->GetData());
			}
			SH_INFO_FORMAT("{}, {}, {}, {}", pixels[0], pixels[1], pixels[2], pixels[3]);
			pickingCallback.Notify({ pixels[0], pixels[1], pixels[2], pixels[3] });
		}
	}

	SH_GAME_API void PickingCamera::SetPickingPos(const Vec2& pos)
	{
		x = pos.x;
		y = pos.y;
	}

	SH_GAME_API void PickingCamera::SetTextureSize(const Vec2& size)
	{
		camera.SetWidth(size.x);
		camera.SetHeight(size.y);
		renderTex->SetSize(size.x, size.y);
	}

	SH_GAME_API void PickingCamera::SetFollowCamera(Camera* follow)
	{
		if (follow == this || !core::IsValid(follow))
			return;
		followCamera = follow;
	}
}//namespace