#pragma once

#include "Export.h"
#include "Shader.h"

#include <memory>

namespace sh::render
{
	class IBuffer;
	class IUniformBuffer;
	class Renderer;
	namespace vk
	{
		class VulkanRenderer;
	}

	class BufferFactory
	{
	private:
		static auto CreateVkUniformBuffer(const vk::VulkanRenderer& renderer, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>;
	public:
		SH_RENDER_API static auto Create(const Renderer& renderer, std::size_t size, bool bTransferDst = false) -> std::unique_ptr<IBuffer>;
		SH_RENDER_API static auto CreateUniformBuffer(const Renderer& renderer, const Shader& shader, Shader::UniformType type) -> std::unique_ptr<IUniformBuffer>;
	};
}//namespace