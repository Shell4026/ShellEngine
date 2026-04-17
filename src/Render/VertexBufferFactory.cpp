#include "VertexBufferFactory.h"
#include "VulkanContext.h"

#include "VulkanImpl/VulkanVertexBuffer.h"
#include "VulkanImpl/VulkanSkinnedVertexBuffer.h"
#include "SkinnedMesh.h"

#include <cassert>
namespace sh::render
{
	auto sh::render::VertexBufferFactory::Create(const IRenderContext& context, const Mesh& mesh) -> std::unique_ptr<IVertexBuffer>
	{
		assert(context.GetRenderAPIType() == RenderAPI::Vulkan);
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto buffer = std::make_unique<vk::VulkanVertexBuffer>(static_cast<const vk::VulkanContext&>(context));
			buffer->Create(mesh);
			return buffer;
		}
		return nullptr;
	}
	auto sh::render::VertexBufferFactory::CreateSkinned(const IRenderContext& context, const SkinnedMesh& mesh) -> std::unique_ptr<IVertexBuffer>
	{
		assert(context.GetRenderAPIType() == RenderAPI::Vulkan);
		if (context.GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto buffer = std::make_unique<vk::VulkanSkinnedVertexBuffer>(static_cast<const vk::VulkanContext&>(context));
			buffer->Create(mesh);
			return buffer;
		}
		return nullptr;
	}
}//namespace