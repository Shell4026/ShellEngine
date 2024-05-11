#include "Material.h"

namespace sh::render
{
	Material::Material() : shader(nullptr)
	{

	}
	Material::Material(Shader* shader) :
		shader(shader)
	{

	}

	void Material::SetShader(Shader* shader)
	{
		this->shader = shader;
	}
	
	auto Material::GetShader() const -> Shader*
	{
		return shader;
	}
}