#pragma once
#include "Export.h"
#include "../ShaderPassBuilder.h"

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanShaderPassBuilder : public ShaderPassBuilder
	{
	private:
		const render::vk::VulkanContext& context;
	public:
		SH_RENDER_API VulkanShaderPassBuilder(const render::vk::VulkanContext& context);
		SH_RENDER_API ~VulkanShaderPassBuilder();

		SH_RENDER_API auto Build() -> std::unique_ptr<render::ShaderPass> override;
	};
}