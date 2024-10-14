#include "pch.h"
#include "Shader.h"

namespace sh::render
{
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

	Shader::Shader(int id, ShaderType type) :
		id(id), type(type) 
	{
		AddAttribute<glm::vec3>("vertex", 0);
	}


	Shader::Shader(Shader&& other) noexcept :
		id(other.id), type(other.type),
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)),
		uniformBindings(std::move(other.uniformBindings)),
		vertexUniforms(std::move(other.vertexUniforms)),
		fragmentUniforms(std::move(other.fragmentUniforms)),
		samplerUniforms(std::move(other.samplerUniforms))
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
		uniformBindings = std::move(other.uniformBindings);
		vertexUniforms = std::move(other.vertexUniforms);
		fragmentUniforms = std::move(other.fragmentUniforms);
		samplerUniforms = std::move(other.samplerUniforms);
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
	auto Shader::GetAttributes() const -> const std::vector<Data>&
	{
		return attrs;
	}
	auto Shader::GetVertexUniforms() const -> const std::vector<UniformBlock>&
	{
		return vertexUniforms;
	}
	auto Shader::GetFragmentUniforms() const -> const std::vector<UniformBlock>&
	{
		return fragmentUniforms;
	}
	auto Shader::GetSamplerUniforms() const -> const std::vector<UniformData>&
	{
		return samplerUniforms;
	}
	auto Shader::GetUniformBinding(std::string_view name) const -> std::optional<uint32_t>
	{
		auto it = uniformBindings.find(std::string{ name });
		if (it == uniformBindings.end())
			return {};
		return it->second;
	}
}