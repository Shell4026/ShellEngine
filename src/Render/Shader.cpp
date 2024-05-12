#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		id(id), type(type), properties(props)
	{
	}

	Shader::Shader(const Shader& other) :
		id(other.id), type(other.type), props(other.props), properties(props)
	{
	}

	Shader::Shader(Shader&& other) noexcept :
		id(other.id), type(other.type), props(std::move(other.props)), properties(props)
	{
	}

	auto Shader::GetShaderType() const -> ShaderType
	{
		return type;
	}

	void Shader::operator=(const Shader& other)
	{
		id = other.id;

		props = other.props;
	}

	void Shader::operator=(Shader&& other) noexcept
	{
		id = other.id;

		props = std::move(other.props);
	}

	auto Shader::operator==(const Shader& other) -> bool
	{
		return id == other.id;
	}

	auto Shader::GetId() const -> int
	{
		return id;
	}

	bool Shader::AddProperty(const std::string& name, uint32_t loc, PropertyType type)
	{
		if (propIdx.find(name) == propIdx.end())
			return false;

		if (props.size() < loc + 1)
			props.resize(loc + 1, PropertyType::None);
		props[loc] = type;
		propIdx.insert({ name, loc });

		return true;
	}

	auto Shader::HasProperty(const std::string& name) const -> bool
	{
		return propIdx.find(name) != propIdx.end();
	}

	auto Shader::GetPropertyType(const std::string& name) const -> std::optional<PropertyType>
	{
		auto it = propIdx.find(name);
		if (it == propIdx.end())
			return {};

		return props[it->second];
	}

	auto Shader::GetPropertyType(uint32_t idx) const -> std::optional<PropertyType>
	{
		if (idx + 1 > props.size())
			return {};
		if (props[idx] == PropertyType::None)
			return {};

		return props[idx];
	}

	auto Shader::GetPropertyIdx(std::string_view name) const -> std::optional<uint32_t>
	{
		auto it = propIdx.find(std::string{ name });
		if (it == propIdx.end())
			return {};
		return it->second;
	}
}