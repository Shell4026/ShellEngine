#include "Material.h"

#include "Core/Util.h"

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

	bool Material::SetVectorArray(std::string_view _name, const std::vector<glm::vec4>& value)
	{
		std::string name{ _name };
		if (!sh::core::IsValid(shader))
			return false;

		auto attr = shader->GetAttribute(name);
		if (!attr)
			return false;

		using Type = Shader::PropertyType;
		if (attr->type != Type::Vec2 && attr->type != Type::Vec3 && attr->type != Type::Vec4)
			return false;

		auto it = vectorArrs.find(name);
		if (it == vectorArrs.end())
			vectorArrs.insert({ name, value });
		else
			it->second = value;

		return true;
	}

	auto Material::GetVectorArray(std::string_view name) -> const std::vector<glm::vec4>*
	{
		auto it = vectorArrs.find(std::string{ name });
		if (it == vectorArrs.end())
			return nullptr;
		return &it->second;
	}
}