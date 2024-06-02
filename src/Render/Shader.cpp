#include "Shader.h"

namespace sh::render
{
	Shader::Shader(int id, ShaderType type) :
		attributes(attrs), vertexUniforms(_vertexUniforms), samplerFragmentUniforms(_samplerFragmentUniforms),

		id(id), type(type) 
	{
		AddAttribute<glm::vec3>("vertex", 0);
	}


	Shader::Shader(Shader&& other) noexcept :
		attributes(attrs), vertexUniforms(_vertexUniforms), samplerFragmentUniforms(_samplerFragmentUniforms),

		id(other.id), type(other.type),
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)),
		_vertexUniforms(std::move(other._vertexUniforms)),
		_fragmentUniforms(std::move(other._fragmentUniforms)),
		_samplerVertexUniforms(std::move(other._samplerVertexUniforms)),
		_samplerFragmentUniforms(std::move(other._samplerFragmentUniforms)),
		topology(other.topology)
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

	Shader::UniformData::UniformData(const UniformData& other) :
		binding(other.binding), offset(other.offset), size(other.size),
		name(other.name), typeName(other.typeName)
	{
	}

	Shader::UniformData::UniformData(UniformData&& other) noexcept :
		binding(other.binding), offset(other.offset), size(other.size),
		name(std::move(other.name)), typeName(other.typeName)
	{
	}

	void Shader::SetTopology(Topology topology)
	{
		this->topology = topology;
	}
	auto Shader::GetTopology() const -> Topology
	{
		return topology;
	}
}