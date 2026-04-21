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
	public:
		struct CreateInfo
		{
			std::size_t size = 0;
			bool bDynamic = false; // SSBO
			bool bTransferDst = false;
		};
	public:
		SH_RENDER_API static auto Create(const IRenderContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>;
		SH_RENDER_API static auto CreateUniformBuffer(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Usage usage) -> std::unique_ptr<IUniformBuffer>;
	private:
		static auto CreateVkUniformBuffer(const vk::VulkanContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>;
	};
}//namespace