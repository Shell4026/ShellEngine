#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		attributes(attrs), uniforms(_uniforms),
		id(id), type(type) 
	{
	}


	Shader::Shader(Shader&& other) noexcept :
		attributes(attrs), uniforms(_uniforms),
		id(other.id), type(other.type), 
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)), 
		_uniforms(std::move(other._uniforms)), uniformIdx(std::move(other.uniformIdx))
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

	bool Shader::HasAttribute(const std::string& name) const
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return false;
		return true;
	}

	auto Shader::GetAttribute(const std::string& name) const -> std::optional<Data>
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return {};
		
		return attrs[it->second];
	}
}