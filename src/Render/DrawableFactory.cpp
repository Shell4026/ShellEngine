//#include "pch.h"
//#include "DrawableFactory.h"
//#include "VulkanContext.h"
//#include "VulkanDrawable.h"
//
//#include "Core/SObject.h"
//
//namespace sh::render
//{
//	auto DrawableFactory::Create(const IRenderContext& context) -> IDrawable*
//	{
//		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
//		{
//			return core::SObject::Create<vk::VulkanDrawable>(static_cast<const vk::VulkanContext&>(context));
//		}
//		return nullptr;
//	}
//}//namespace