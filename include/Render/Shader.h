#pragma once

#include "Export.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <unordered_map>
#include <string>
#include <optional>

namespace sh::render
{
	class Shader : public sh::core::SObject
	{
		SCLASS(Shader)
	public:
		enum class ShaderType
		{
			GLSL,
			SPIR,
		};

		enum class PropertyType
		{
			Int, Float,
			Vec2, Vec3, Vec4
		};
	private:
		std::unordered_map<std::string, PropertyType> _properties;
	protected:
		int id;

		ShaderType type;
	protected:
		Shader(int id, ShaderType type);
		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
	public:
		const std::unordered_map<std::string, PropertyType>& properties;
	public:
		SH_RENDER_API void operator=(const Shader& other);
		SH_RENDER_API void operator=(Shader&& other) noexcept;
		SH_RENDER_API auto operator==(const Shader& other) -> bool;
		SH_RENDER_API auto GetShaderType() const -> ShaderType;

		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API void AddProperty(const std::string& name, PropertyType type);
		SH_RENDER_API auto HasProperty(const std::string& name) const -> bool;
		SH_RENDER_API auto GetProperty(const std::string& name) const -> std::optional<PropertyType>;

		SH_RENDER_API auto GetId() const -> int;
	};
}