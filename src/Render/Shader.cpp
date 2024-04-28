#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		id(id), type(type), properties(_properties)
	{
	}

	Shader::Shader(const Shader& other) :
		id(other.id), type(other.type), _properties(other._properties), properties(_properties)
	{
	}

	Shader::Shader(Shader&& other) noexcept :
		id(other.id), type(other.type), _properties(std::move(other._properties)), properties(_properties)
	{
	}

	auto Shader::GetShaderType() const -> ShaderType
	{
		return type;
	}

	void Shader::operator=(const Shader& other)
	{
		id = other.id;

		_properties = other._properties;
	}

	void Shader::operator=(Shader&& other) noexcept
	{
		id = other.id;

		_properties = std::move(other._properties);
	}

	auto Shader::operator==(const Shader& other) -> bool
	{
		return id == other.id;
	}

	auto Shader::GetId() const -> int
	{
		return id;
	}

	void Shader::AddProperty(const std::string& name, PropertyType type)
	{
		_properties.insert({ name, type });
	}

	auto Shader::HasProperty(const std::string& name) const -> bool
	{
		return _properties.find(name) != _properties.end();
	}

	auto Shader::GetProperty(const std::string& name) const -> std::optional<PropertyType>
	{
		auto it = _properties.find(name);
		if (it == _properties.end())
			return {};

		return it->second;
	}
}