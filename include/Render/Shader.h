#pragma once

#include "Export.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <unordered_map>
#include <string>
#include <optional>

namespace sh::render
{
	class Shader : public sh::core::SObject, sh::core::INonCopyable
	{
		SCLASS(Shader)
	public:
		enum class ShaderType
		{
			GLSL,
			SPIR,
		};

		struct Data
		{
			uint32_t idx;
			std::string_view typeName;
			std::string name;
			size_t size;
		};
	private:
		std::vector<Data> attrs;
		std::unordered_map<std::string, uint32_t> attridx;
		std::vector<Data> _uniforms;
		std::unordered_map<std::string, uint32_t> uniformIdx;
	protected:
		int id;

		ShaderType type;
	protected:
		Shader(int id, ShaderType type);
		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
	public:
		const std::vector<Data>& attributes;
		const std::vector<Data>& uniforms;
	public:
		SH_RENDER_API virtual ~Shader() = default;
		SH_RENDER_API void operator=(Shader&& other) noexcept;
		SH_RENDER_API auto operator==(const Shader& other) -> bool;
		SH_RENDER_API auto GetShaderType() const -> ShaderType;

		SH_RENDER_API virtual void Clean() = 0;

		template<typename T>
		bool AddAttribute(const std::string& name, uint32_t loc);
		SH_RENDER_API bool HasAttribute(const std::string& name) const;
		SH_RENDER_API auto GetAttribute(const std::string& name) const -> std::optional<Data>;

		template<typename T>
		bool AddUniform(const std::string& name, uint32_t loc);
		SH_RENDER_API bool HasUniform(const std::string& name) const;
		SH_RENDER_API auto GetUniform(const std::string& name) const->std::optional<Data>;

		SH_RENDER_API auto GetId() const -> int;
	};

	template<typename T>
	inline bool Shader::AddAttribute(const std::string& name, uint32_t loc)
	{
		auto it = attridx.find(name);
		if (it != attridx.end())
			return false;

		if (attrs.size() < loc + 1)
		{
			attrs.resize(loc + 1);
		}
		Data attr;
		attr.idx = loc;
		attr.name = name;
		attr.typeName = sh::core::reflection::GetTypeName<T>();
		attr.size = sizeof(T);

		attrs[loc] = attr;
		attridx.insert({ name, loc });

		return true;
	}

	template<typename T>
	inline bool Shader::AddUniform(const std::string& name, uint32_t loc)
	{
		auto it = uniformIdx.find(name);
		if (it != uniformIdx.end())
			return false;

		if (_uniforms.size() < loc + 1)
		{
			_uniforms.resize(loc + 1);
		}
		Data uniform;
		uniform.idx = loc;
		uniform.name = name;
		uniform.typeName = sh::core::reflection::GetTypeName<T>();
		uniform.size = sizeof(T);
		_uniforms[loc] = uniform;
		uniformIdx.insert({ name, loc });

		return true;
	}
}