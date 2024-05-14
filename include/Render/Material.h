#pragma once

#include "Export.h"
#include "Shader.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

#include <string_view>
#include <vector>
#include <Utility>

namespace sh::render
{
	class Material : public sh::core::SObject
	{
		SCLASS(Material)
	private:
		PROPERTY(shader)
		Shader* shader;

		std::unordered_map<std::string, std::vector<glm::vec4>> vectorArrs;
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;

		bool SetVectorArray(std::string_view name, const std::vector<glm::vec4>& value);
		auto GetVectorArray(std::string_view name) -> const std::vector<glm::vec4>*;
	};
}