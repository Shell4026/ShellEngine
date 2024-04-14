#include "Shader.h"

namespace sh::render
{
	Shader::Shader(ShaderType type) :
		type(type)
	{
	}

	auto Shader::GetShaderType() -> ShaderType
	{
		return type;
	}
}