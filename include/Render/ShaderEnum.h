#pragma once

namespace sh::render
{
	enum class ShaderType
	{
		GLSL,
		SPIR,
	};
	enum class ShaderStage
	{
		Vertex,
		Fragment
	};
	enum class CullMode
	{
		Off,
		Front,
		Back,
	};
}