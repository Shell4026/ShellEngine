#pragma once

#include "Export.h"
#include "Shader.h"

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
		const Renderer& renderer;

		PROPERTY(shader)
		Shader* shader;

		std::unordered_map<std::string, glm::vec4> vectors;
		std::unordered_map<std::string, std::vector<glm::vec4>> vectorArrs;
	public:
		SH_RENDER_API Material(const Renderer& renderer);
		SH_RENDER_API Material(const Renderer& renderer, Shader* shader);
		SH_RENDER_API ~Material();
		SH_RENDER_API Material(Material&& other) noexcept;

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;

		SH_RENDER_API bool SetVector(std::string_view name, const glm::vec4& value);
		SH_RENDER_API auto GetVector(std::string_view name) -> glm::vec4*;
		SH_RENDER_API bool SetVectorArray(std::string_view name, const std::vector<glm::vec4>& value);
		SH_RENDER_API auto GetVectorArray(std::string_view name) -> const std::vector<glm::vec4>*;
	};
}