#include "Component/PickingCamera.h"

#include "Game/World.h"
#include "Game/GameObject.h"

#include <chrono>
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
		render::RenderTargetLayout rt{};
		rt.format = render::TextureFormat::RGBA32;
		rt.depthFormat = render::TextureFormat::D24S8;
		rt.bUseMSAA = false;

		renderTex = core::SObject::Create<render::RenderTexture>(rt);
		renderTex->SetSize(1024, 768);
		renderTex->Build(*world.renderer.GetContext());
		camera.SetWidth(1024);
		camera.SetHeight(768);
		renderTex->SetName("PickingFramebuffer");

		SetRenderTexture(renderTex);
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
		if (!pickingCallback.Empty() && !bRequestRead)
		{
			bufferFuture = world.renderer.GetScriptableRenderer()->ReadRenderTextureAsync(*renderTex, x, y);
			bRequestRead = true;
		}

		if (bufferFuture.valid())
		{
			auto status = bufferFuture.wait_for(std::chrono::milliseconds(0));
			if (status == std::future_status::ready)
			{
				pixels = reinterpret_cast<uint8_t*>(bufferFuture.get()->GetData());
				SH_INFO_FORMAT("{}, {}, {}, {}", pixels[0], pixels[1], pixels[2], pixels[3]);

				pickingCallback.Notify({ pixels[0], pixels[1], pixels[2], pixels[3] });
				bRequestRead = false;
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