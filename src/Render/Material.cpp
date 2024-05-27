#include "Material.h"

#include "VulkanRenderer.h"
#include "VulkanUniform.h"

#include "Core/Util.h"

namespace sh::render
{
	Material::Material() : 
		shader(nullptr)
	{
	}
	Material::Material(Shader* shader) :
		shader(shader)
	{
	}

	Material::~Material()
	{
	}

	Material::Material(Material&& other) noexcept :
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

	bool Material::SetFloat(std::string_view _name, float value)
	{
		std::string name{ _name };

		auto it = floats.find(name);
		if (it == floats.end())
			floats.insert({ name, value });
		else
			it->second = value;

		return true;
	}

	auto Material::GetFloat(std::string_view _name) -> float
	{
		auto it = floats.find(std::string{ _name });
		if (it == floats.end())
			return 0.f;
		return it->second;
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
	auto Material::GetVector(std::string_view name) -> const glm::vec4*
	{
		auto it = vectors.find(std::string{ name });
		if (it == vectors.end())
			return nullptr;
		return &it->second;
	}

	bool Material::SetMatrix(std::string_view _name, const glm::mat4& value)
	{
		std::string name{ _name };

		auto it = mats.find(name);
		if (it == mats.end())
			mats.insert({ name, value });
		else
			it->second = value;

		return true;
	}
	auto Material::GetMatrix(std::string_view _name) -> const glm::mat4*
	{
		auto it = mats.find(std::string{ _name });
		if (it == mats.end())
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

	bool Material::SetTexture(std::string_view _name, Texture* tex)
	{
		std::string name{ _name };

		auto it = textures.find(name);
		if (it == textures.end())
			textures.insert({ name, tex });
		else
			it->second = tex;

		return true;
	}
	auto Material::GetTexture(std::string_view name) -> Texture*
	{
		auto it = textures.find(std::string{ name });
		if (it == textures.end())
			return nullptr;
		return it->second;
	}
}