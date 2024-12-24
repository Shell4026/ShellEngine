#include "pch.h"
#include "DrawableFactory.h"
#include "VulkanRenderer.h"
#include "VulkanDrawable.h"

#include "Core/SObject.h"

namespace sh::render
{
	auto DrawableFactory::Create(Renderer& renderer) -> IDrawable*
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			return core::SObject::Create<vk::VulkanDrawable>(static_cast<vk::VulkanRenderer&>(renderer));
		}
		return nullptr;
	}
}//namespace