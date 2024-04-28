#pragma once

#include "Export.h"

#include "Core/Reflaction.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <unordered_map>
#include <string>
#include <optional>

namespace sh::render
{
	class Shader
	{
		SCLASS(Shader)
	public:
		enum class ShaderType
		{
			GLSL,
			SPIR,
		};

		struct Property
		{
			enum class Type
			{
				Int, Float,
				Vec2, Vec3, Vec4
			};
			Type type;

			union Data
			{
				int intData;
				float floatData;
				glm::vec2 vec2Data;
				glm::vec3 vec3Data;
				glm::vec4 vec4Data;
			};
			Data data;

			SH_RENDER_API Property();
			SH_RENDER_API Property(const Property& other);

			SH_RENDER_API void operator=(const Property& other);
		};
	private:
		std::unordered_map<std::string, Property> properties;
	protected:
		int id;

		ShaderType type;
	protected:
		Shader(int id, ShaderType type);
		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
	public:
		SH_RENDER_API void operator=(const Shader& other);
		SH_RENDER_API void operator=(Shader&& other) noexcept;
		SH_RENDER_API auto operator==(const Shader& other) -> bool;
		SH_RENDER_API auto GetShaderType() const -> ShaderType;

		SH_RENDER_API virtual void Clean() = 0;

		SH_RENDER_API void AddProperty(const std::string& name);

		SH_RENDER_API void SetProperty(const std::string& name, int value);
		SH_RENDER_API void SetProperty(const std::string& name, float value);
		SH_RENDER_API void SetProperty(const std::string& name, const glm::vec2& value);
		SH_RENDER_API void SetProperty(const std::string& name, const glm::vec3& value);
		SH_RENDER_API void SetProperty(const std::string& name, const glm::vec4& value);

		SH_RENDER_API auto GetProperty(const std::string& name) const -> std::optional<Property>;

		SH_RENDER_API auto GetId() const -> int;
	};
}