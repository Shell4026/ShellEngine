#pragma once

#include "Export.h"
#include "Shader.h"
#include "Texture.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

#include "glm/mat4x4.hpp"

#include <string_view>
#include <vector>
#include <unordered_map>

namespace sh::render
{
	class Renderer;

	class Material : public sh::core::SObject
	{
		SCLASS(Material)
	private:
		PROPERTY(shader)
		Shader* shader;

		std::unordered_map<std::string, float> floats;
		std::unordered_map<std::string, glm::vec4> vectors;
		std::unordered_map<std::string, glm::mat4> mats;
		std::unordered_map<std::string, std::vector<glm::vec4>> vectorArrs;
		PROPERTY(textures)
		std::unordered_map<std::string, Texture*> textures;
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);
		SH_RENDER_API ~Material();
		SH_RENDER_API Material(Material&& other) noexcept;

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const->Shader*;

		SH_RENDER_API bool SetFloat(std::string_view name, float value);
		SH_RENDER_API auto GetFloat(std::string_view name) -> float;
		SH_RENDER_API bool SetVector(std::string_view name, const glm::vec4& value);
		SH_RENDER_API auto GetVector(std::string_view name) -> const glm::vec4*;
		SH_RENDER_API bool SetMatrix(std::string_view name, const glm::mat4& value);
		SH_RENDER_API auto GetMatrix(std::string_view name) -> const glm::mat4*;
		SH_RENDER_API bool SetVectorArray(std::string_view name, const std::vector<glm::vec4>& value);
		SH_RENDER_API auto GetVectorArray(std::string_view name) -> const std::vector<glm::vec4>*;
		SH_RENDER_API bool SetTexture(std::string_view name, Texture* tex);
		SH_RENDER_API auto GetTexture(std::string_view name) -> Texture*;
	};
}