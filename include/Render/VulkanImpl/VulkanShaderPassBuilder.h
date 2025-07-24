#pragma once
#include "Export.h"
#include "../ShaderPassBuilder.h"

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanShaderPassBuilder : public ShaderPassBuilder
	{
	private:
		const VulkanContext& context;
	public:
		SH_RENDER_API VulkanShaderPassBuilder(const render::vk::VulkanContext& context);
		SH_RENDER_API ~VulkanShaderPassBuilder();

		SH_RENDER_API auto GetContext() const -> const VulkanContext&;

		SH_RENDER_API auto Build(const ShaderAST::PassNode& passNode) -> render::ShaderPass* override;
	};
}