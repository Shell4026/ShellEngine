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

	bool Shader::AddAttribute(const std::string& name, uint32_t loc, DataType type)
	{
		auto it = attridx.find(name);
		if (it != attridx.end())
			return false;

		if (attrs.size() < loc + 1)
		{
			attrs.resize(loc + 1);
		}
		Data attr;
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

	auto Shader::GetAttribute(const std::string& name) const -> std::optional<Data>
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return {};
		
		return attrs[it->second];
	}

	bool Shader::AddUniform(const std::string& name, uint32_t loc, DataType type)
	{
		auto it = uniformIdx.find(name);
		if (it != uniformIdx.end())
			return false;

		if (_uniforms.size() < loc + 1)
		{
			_uniforms.resize(loc + 1);
		}
		Data uniform;
		uniform.idx = loc;
		uniform.name = name;
		uniform.type = type;

		_uniforms[loc] = uniform;
		uniformIdx.insert({ name, loc });

		return true;
	}

	bool Shader::HasUniform(const std::string& name) const
	{
		auto it = uniformIdx.find(name);
		if (it == uniformIdx.end())
			return false;
		return true;
	}

	auto Shader::GetUniform(const std::string& name) const -> std::optional<Data>
	{
		auto it = uniformIdx.find(name);
		if (it == uniformIdx.end())
			return {};

		return _uniforms[it->second];
	}
}