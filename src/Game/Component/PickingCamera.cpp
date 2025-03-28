#include "Component/PickingCamera.h"

#include "Game/World.h"
#include "Game/GameObject.h"
#include "Game/RenderThread.h"

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
		renderTex = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::RGBA32);
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
			if (!addTask)
			{
				RenderThread::GetInstance()->AddEndTaskFromOtherThread([&]
					{
						renderFinished.store(true, std::memory_order::memory_order_release);
					}
				);
				addTask = true;
			}
			else
			{
				if (renderFinished.load(std::memory_order::memory_order_acquire))
				{
					// 요청을 받은 후 적어도 1프레임 뒤에 알 수 있다.
					// (에디터 요청 -> 렌더링 요청) -> 스레드 동기화 -> 렌더링 -> renderFinsihed에 신호 전달 - 0프레임
					// 신호를 받았으면 처리, 아니면 다음 프레임 - 1프레임
					assert(world.renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan);
					if (world.renderer.GetContext()->GetRenderAPIType() == render::RenderAPI::Vulkan)
					{
						auto vkFramebuffer = static_cast<render::vk::VulkanFramebuffer*>(renderTex->GetFramebuffer(core::ThreadType::Render));
						auto& vkContext = static_cast<render::vk::VulkanContext&>(*world.renderer.GetContext());
						auto vkBuffer = static_cast<render::vk::VulkanBuffer*>(buffer.get());

						vkFramebuffer->TransferImageToBuffer(vkContext.GetCommandBuffer(core::ThreadType::Game), vkBuffer->GetBuffer(), x, y);
						pixels = reinterpret_cast<uint8_t*>(buffer->GetData());
					}

					pickingCallback.Notify({ pixels[0], pixels[1], pixels[2], pixels[3] });
					addTask = false;
				}
			}
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