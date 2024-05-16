#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		id(id), type(type), attributes(attrs)
	{
	}


	Shader::Shader(Shader&& other) noexcept :
		id(other.id), type(other.type), attrs(std::move(other.attrs)), attridx(other.attridx), attributes(attrs)
	{
	}

	auto Shader::GetShaderType() const -> ShaderType
	{
		return type;
	}

	void Shader::operator=(Shader&& other) noexcept
	{
		id = other.id;

		attrs = std::move(other.attrs);
		attridx = std::move(other.attridx);
	}

	auto Shader::operator==(const Shader& other) -> bool
	{
		return id == other.id;
	}

	auto Shader::GetId() const -> int
	{
		return id;
	}

	bool Shader::AddAttribute(const std::string& name, uint32_t loc, AttributeType type)
	{
		auto it = attridx.find(name);
		if (it != attridx.end())
			return false;

		if (attrs.size() < loc + 1)
		{
			attrs.resize(loc + 1);
		}
		Attribute attr;
		attr.idx = loc;
		attr.name = name;
		attr.type = type;

		attrs[loc] = attr;
		attridx.insert({ name, loc });

		return true;
	}

	bool Shader::HasAttribute(const std::string& name) const
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return false;
		return true;
	}

	auto Shader::GetAttribute(const std::string& name) const -> std::optional<Attribute>
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return {};
		
		return attrs[it->second];
	}
}