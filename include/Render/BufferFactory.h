#pragma once

#include "Export.h"
#include "ShaderPass.h"

#include <memory>

namespace sh::render
{
	class IBuffer;
	class IShaderBinding;
	class IRenderContext;
	class ComputeShader;
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
			bool bGPUOnly = false;
		};
	public:
		SH_RENDER_API static auto Create(const IRenderContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>;
		SH_RENDER_API static auto CreateShaderBinding(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Usage usage) -> std::unique_ptr<IShaderBinding>;
		SH_RENDER_API static auto CreateShaderBinding(const IRenderContext& context, const ComputeShader& shader) -> std::unique_ptr<IShaderBinding>;

		SH_RENDER_API static auto GetBufferAlignment(const IRenderContext& context) -> std::size_t;
	private:
		static auto CreateVkUniformBuffer(const vk::VulkanContext& context, const CreateInfo& info) -> std::unique_ptr<IBuffer>;
	};
}//namespace