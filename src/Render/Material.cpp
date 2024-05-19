#include "Material.h"

#include "VulkanRenderer.h"
#include "VulkanUniform.h"

#include "Core/Util.h"

namespace sh::render
{
	Material::Material(const Renderer& renderer) : 
		renderer(renderer),
		shader(nullptr)
	{
	}
	Material::Material(const Renderer& renderer, Shader* shader) :
		renderer(renderer),
		shader(shader)
	{
	}

	Material::~Material()
	{
	}

	Material::Material(Material&& other) noexcept :
		renderer(other.renderer),
		shader(other.shader),
		vectors(std::move(other.vectors)),
		vectorArrs(std::move(other.vectorArrs))
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

	bool Material::SetVector(std::string_view _name, const glm::vec4& value)
	{
		std::string name{ _name };

		auto it = vectors.find(name);
		if (it == vectors.end())
			vectors.insert({ name, value });
		else
			it->second = value;

		return true;
	}

	auto Material::GetVector(std::string_view name) -> glm::vec4*
	{
		auto it = vectors.find(std::string{ name });
		if (it == vectors.end())
			return nullptr;
		return &it->second;
	}

	bool Material::SetVectorArray(std::string_view _name, const std::vector<glm::vec4>& value)
	{
		std::string name{ _name };

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