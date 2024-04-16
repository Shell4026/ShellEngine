#pragma once

#include "Export.h"

#include "Core/Reflaction.hpp"

namespace sh::render
{
	class Shader
	{
		SCLASS(Shader)
	public:
		enum class ShaderType
		{
			GLSL,
			SPIR,
		};
	protected:
		ShaderType type;
	protected:
		Shader(ShaderType type);
	public:
		SH_RENDER_API auto GetShaderType()->ShaderType;

		SH_RENDER_API virtual void Clean() = 0;
	};
}