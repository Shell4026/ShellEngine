#pragma once

#include "ShaderBuilder.h"

#include "Export.h"

#include <vector>

namespace sh::render
{
	class VulkanRenderer;
}
namespace sh::game
{
	class VulkanShaderBuilder : public ShaderBuilder
	{
	private:
		render::VulkanRenderer& renderer;
	public:
		SH_GAME_API VulkanShaderBuilder(render::VulkanRenderer& renderer);
		SH_GAME_API ~VulkanShaderBuilder();

		SH_GAME_API auto Build()->std::unique_ptr<render::Shader> override;
	};
}