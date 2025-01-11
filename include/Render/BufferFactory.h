#pragma once

#include "Export.h"
#include "ShaderPass.h"

#include <memory>

namespace sh::render
{
	class IBuffer;
	class IUniformBuffer;
	class IRenderContext;
	namespace vk
	{
		class VulkanContext;
	}

	class BufferFactory
	{
	private:
		static auto CreateVkUniformBuffer(const vk::VulkanContext& context, std::size_t size, bool bTransferDst) -> std::unique_ptr<IBuffer>;
	public:
		SH_RENDER_API static auto Create(const IRenderContext& context, std::size_t size, bool bTransferDst = false) -> std::unique_ptr<IBuffer>;
		SH_RENDER_API static auto CreateUniformBuffer(const IRenderContext& context, const ShaderPass& shader, ShaderPass::UniformType type) -> std::unique_ptr<IUniformBuffer>;
	};
}//namespace