#pragma once

#include "ShaderBuilder.h"

#include "Export.h"

namespace sh::render::vk
{
	class VulkanContext;
}
namespace sh::game
{
	class VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		const render::vk::VulkanContext& context;
	public:
		SH_GAME_API VulkanShaderBuilder(const render::vk::VulkanContext& context);
		SH_GAME_API ~VulkanShaderBuilder();

		SH_GAME_API auto Build() -> render::Shader* override;
	};
}