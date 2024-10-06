#include "pch.h"
#include "VertexBufferFactory.h"
#include "VulkanRenderer.h"

#include "VulkanImpl/VulkanVertexBuffer.h"

namespace sh::render
{
	auto sh::render::VertexBufferFactory::Create(const Renderer& renderer, const Mesh& mesh) -> std::unique_ptr<IVertexBuffer>
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			auto buffer = std::make_unique<VulkanVertexBuffer>(static_cast<const VulkanRenderer&>(renderer));
			buffer->Create(mesh);
			return buffer;
		}
		return nullptr;
	}
}//namespace