#include "PCH.h"
#include "Component/PickingCamera.h"

#include "Game/World.h"
#include "Game/GameObject.h"

#include "Render/VulkanImpl/VulkanFramebuffer.h"
#include "Render/VulkanImpl/VulkanImageBuffer.h"
#include "Render/VulkanImpl/VulkanRenderer.h"
#include "Render/VulkanImpl/VulkanBuffer.h"
#include "Render/BufferFactory.h"


namespace sh::game
{
	PickingCamera::PickingCamera(GameObject& owner) :
		Camera(owner)
	{
		renderTex = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::RGBA32);
		renderTex->SetReadUsage(true);
		renderTex->SetSize(1024, 768);
		renderTex->Build(world.renderer);
		screenSize = { 1024, 768 };
#if SH_EDITOR
		renderTex->editorName = "PickingFramebuffer";
#endif

		SetRenderTexture(*renderTex);

		buffer = render::BufferFactory::Create(world.renderer, sizeof(uint8_t) * 4, true);
	}

	SH_GAME_API void PickingCamera::Awake()
	{
	}

	SH_GAME_API void PickingCamera::BeginUpdate()
	{
		if (!pickingCallback.Empty())
		{
			if (core::IsValid(followCamera))
			{
				if (&gameObject != &followCamera->gameObject)
					gameObject.transform->SetPosition(followCamera->gameObject.transform->position);
				this->SetLookPos(followCamera->GetLookPos());
				this->SetUpVector(followCamera->GetUpVector());
			}
			CalcMatrix();
			assert(world.renderer.apiType == render::RenderAPI::Vulkan);
			if (world.renderer.apiType == render::RenderAPI::Vulkan)
			{
				auto vkFramebuffer = static_cast<render::impl::VulkanFramebuffer*>(renderTex->GetFramebuffer(core::ThreadType::Game));
				auto& vkRenderer = static_cast<render::VulkanRenderer&>(world.renderer);
				auto vkBuffer = static_cast<render::impl::VulkanBuffer*>(buffer.get());

				vkFramebuffer->TransferImageToBuffer(vkRenderer.GetCommandBuffer(core::ThreadType::Game), vkRenderer.GetTransferQueue(), vkBuffer->GetBuffer(), x, y);
				pixels = reinterpret_cast<uint8_t*>(buffer->GetData());
			}
		}
		if (!pickingCallback.Empty())
		{
			if(frameCount == 1)
			{
				pickingCallback.Notify(pixels[0], pixels[1], pixels[2], pixels[3]);
				frameCount = 0;
			}
			else
				++frameCount;
		}
	}

	SH_GAME_API void PickingCamera::SetPickingPos(const Vec2& pos)
	{
		x = pos.x;
		y = pos.y;
	}

	SH_GAME_API void PickingCamera::SetTextureSize(const Vec2& size)
	{
		screenSize = size;
		renderTex->SetSize(size.x, size.y);
	}

	SH_GAME_API void PickingCamera::SetFollowCamera(Camera* follow)
	{
		if (follow == this || !core::IsValid(follow))
			return;
		followCamera = follow;
	}
}//namespace