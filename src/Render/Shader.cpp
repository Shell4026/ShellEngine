#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		id(id), type(type)
	{
	}

	Shader::Shader(const Shader& other) :
		id(other.id), type(other.type), properties(other.properties)
	{
	}

	Shader::Shader(Shader&& other) noexcept :
		id(other.id), type(other.type), properties(std::move(other.properties))
	{
	}

	auto Shader::GetShaderType() const -> ShaderType
	{
		return type;
	}

	Shader::Property::Property() :
		type(Property::Type::Int), data()
	{
	}

	Shader::Property::Property(const Property& other) :
		type(other.type), data(other.data)
	{
	}

	void Shader::Property::operator=(const Property& other)
	{
		type = other.type;
		data = other.data;
	}

	void Shader::operator=(const Shader& other)
	{
		id = other.id;

		properties = other.properties;
	}

	void Shader::operator=(Shader&& other) noexcept
	{
		id = other.id;

		properties = std::move(other.properties);
	}

	auto Shader::operator==(const Shader& other) -> bool
	{
		return id == other.id;
	}

	auto Shader::GetId() const -> int
	{
		return id;
	}

	void Shader::AddProperty(const std::string& name)
	{
		properties.insert({ name, Property{} });
	}

	void Shader::SetProperty(const std::string& name, int value)
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return;

		it->second.type = Property::Type::Int;
		it->second.data.intData = value;
	}

	void Shader::SetProperty(const std::string& name, float value)
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return;

		it->second.type = Property::Type::Float;
		it->second.data.floatData = value;
	}

	void Shader::SetProperty(const std::string& name, const glm::vec2& value)
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return;

		it->second.type = Property::Type::Vec2;
		it->second.data.vec2Data = value;
	}

	void Shader::SetProperty(const std::string& name, const glm::vec3& value)
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return;

		it->second.type = Property::Type::Vec3;
		it->second.data.vec3Data = value;
	}

	void Shader::SetProperty(const std::string& name, const glm::vec4& value)
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return;

		it->second.type = Property::Type::Vec4;
		it->second.data.vec4Data = value;
	}

	auto Shader::GetProperty(const std::string& name) const -> std::optional<Property>
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return {};

		return it->second;
	}
}