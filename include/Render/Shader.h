#pragma once

#include "Export.h"

#include "Core/Reflaction.hpp"

namespace sh::render
{
	class SH_RENDER_API Shader
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
		auto GetShaderType()->ShaderType;

		virtual void Clean() = 0;
	};
}