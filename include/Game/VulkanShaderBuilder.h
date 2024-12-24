#pragma once

#include "ShaderBuilder.h"

#include "Export.h"

#include <vector>

namespace sh::render::vk
{
	class VulkanRenderer;
}
namespace sh::game
{
	class VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		render::vk::VulkanRenderer& renderer;
	public:
		SH_GAME_API VulkanShaderBuilder(render::vk::VulkanRenderer& renderer);
		SH_GAME_API ~VulkanShaderBuilder();

		SH_GAME_API auto Build()->render::Shader* override;
	};
}