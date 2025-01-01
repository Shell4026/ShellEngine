#include "pch.h"
#include "VertexBufferFactory.h"
#include "VulkanContext.h"

#include "VulkanImpl/VulkanVertexBuffer.h"

namespace sh::render
{
	auto sh::render::VertexBufferFactory::Create(const IRenderContext& context, const Mesh& mesh) -> std::unique_ptr<IVertexBuffer>
	{
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto buffer = std::make_unique<vk::VulkanVertexBuffer>(static_cast<const vk::VulkanContext&>(context));
			buffer->Create(mesh);
			return buffer;
		}
		return nullptr;
	}
}//namespace