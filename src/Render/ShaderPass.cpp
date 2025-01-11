#include "ShaderPass.h"

namespace sh::render
{
	ShaderPass::ShaderPass(int id, ShaderType type) :
		id(id), type(type) 
	{
	}


	ShaderPass::ShaderPass(ShaderPass&& other) noexcept :
		id(other.id), type(other.type),
		attrs(std::move(other.attrs)), attridx(std::move(other.attridx)),
		uniformBindings(std::move(other.uniformBindings)),
		vertexUniforms(std::move(other.vertexUniforms)),
		fragmentUniforms(std::move(other.fragmentUniforms)),
		samplerUniforms(std::move(other.samplerUniforms))
	{
	}

	auto ShaderPass::GetShaderType() const -> ShaderType
	{
		return type;
	}

	void ShaderPass::operator=(ShaderPass&& other) noexcept
	{
		id = other.id;

		attrs = std::move(other.attrs);
		attridx = std::move(other.attridx);
		uniformBindings = std::move(other.uniformBindings);
		vertexUniforms = std::move(other.vertexUniforms);
		fragmentUniforms = std::move(other.fragmentUniforms);
		samplerUniforms = std::move(other.samplerUniforms);
	}

	auto ShaderPass::operator==(const ShaderPass& other) -> bool
	{
		return id == other.id;
	}

	SH_RENDER_API void ShaderPass::SetStencilState(StencilState stencilState)
	{
		this->stencilState = stencilState;
	}

	SH_RENDER_API auto ShaderPass::GetStencilState() const -> const StencilState&
	{
		return stencilState;
	}

	auto ShaderPass::GetId() const -> int
	{
		return id;
	}

	bool ShaderPass::HasAttribute(const std::string& name) const
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return false;
		return true;
	}

	auto ShaderPass::GetAttribute(const std::string& name) const -> std::optional<Data>
	{
		auto it = attridx.find(name);
		if (it == attridx.end())
			return {};
		
		return attrs[it->second];
	}
	auto ShaderPass::GetAttributes() const -> const std::vector<Data>&
	{
		return attrs;
	}
	auto ShaderPass::GetVertexUniforms() const -> const std::vector<UniformBlock>&
	{
		return vertexUniforms;
	}
	auto ShaderPass::GetFragmentUniforms() const -> const std::vector<UniformBlock>&
	{
		return fragmentUniforms;
	}
	auto ShaderPass::GetSamplerUniforms() const -> const std::vector<UniformData>&
	{
		return samplerUniforms;
	}
	auto ShaderPass::GetUniformBinding(const std::string& name) const -> std::optional<uint32_t>
	{
		auto it = uniformBindings.find(name);
		if (it == uniformBindings.end())
			return {};
		return it->second;
	}
}