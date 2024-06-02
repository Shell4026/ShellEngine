#pragma once

#include "Export.h"
#include "Texture.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <map>
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

		enum class ShaderStage
		{
			Vertex,
			Fragment
		};

		enum class Topology
		{
			Point,
			Line,
			Triangle
		};

		struct Data
		{
			uint32_t idx;
			std::string_view typeName;
			std::string name;
			size_t size;
		};

		struct UniformData
		{
			uint32_t binding;
			std::string name;
			std::string_view typeName;
			size_t offset;
			size_t size;

			UniformData() = default;
			SH_RENDER_API UniformData(const UniformData& other);
			SH_RENDER_API UniformData(UniformData&& other) noexcept;
		};
	private:
		std::vector<Data> attrs;
		std::unordered_map<std::string, uint32_t> attridx;
		std::map<uint32_t, std::vector<UniformData>> _vertexUniforms;
		std::map<uint32_t, std::vector<UniformData>> _fragmentUniforms;
		std::map<uint32_t, UniformData> _samplerVertexUniforms;
		std::map<uint32_t, UniformData> _samplerFragmentUniforms;

		ShaderType type;
		Topology topology = Topology::Triangle;
	protected:
		int id;
	protected:
		Shader(int id, ShaderType type);
		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
	public:
		const std::vector<Data>& attributes;
		const std::map<uint32_t, std::vector<UniformData>>& vertexUniforms;
		const std::map<uint32_t, UniformData>& samplerFragmentUniforms;
	public:
		SH_RENDER_API virtual ~Shader() = default;
		SH_RENDER_API void operator=(Shader&& other) noexcept;
		SH_RENDER_API auto operator==(const Shader& other) -> bool;
		SH_RENDER_API auto GetShaderType() const -> ShaderType;

		SH_RENDER_API void SetTopology(Topology topology);
		SH_RENDER_API auto GetTopology() const -> Topology;

		SH_RENDER_API virtual void Clean() = 0;
		SH_RENDER_API virtual void Build() = 0;

		template<typename T>
		bool AddAttribute(const std::string& name, uint32_t loc);
		SH_RENDER_API bool HasAttribute(const std::string& name) const;
		SH_RENDER_API auto GetAttribute(const std::string& name) const -> std::optional<Data>;

		template<typename T>
		void AddUniform(const std::string& name, uint32_t binding, ShaderStage stage);

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

	//
	//유니폼을 추가하는 함수
	//반드시 셰이더의 유니폼과 같은 순서로 호출하여 넣을 것.
	//
	template<typename T>
	inline void Shader::AddUniform(const std::string& name, uint32_t binding, ShaderStage stage)
	{
		UniformData uniform;
		uniform.binding = binding;
		uniform.name = name;
		uniform.typeName = sh::core::reflection::GetTypeName<T>();
		uniform.size = sizeof(T);
		uniform.offset = 0;

		std::map<uint32_t, std::vector<UniformData>>::iterator it;
		if (stage == ShaderStage::Vertex)
		{ 
			it = _vertexUniforms.find(binding);
			if (it == _vertexUniforms.end())
			{
				_vertexUniforms.insert({ binding, std::vector<UniformData>{ std::move(uniform) } });
				return;
			}
		}
		else if (stage == ShaderStage::Fragment)
		{
			it = _fragmentUniforms.find(binding);
			if (it == _fragmentUniforms.end())
			{
				_fragmentUniforms.insert({ binding, std::vector<UniformData>{ std::move(uniform) } });
				return;
			}
		}
		/*
		16바이트 메모리 정렬을 맞추는 코드
		*/
		uint32_t next = it->second.back().offset + it->second.back().size;
		uint32_t emptySpace = 16 - next % 16;
		if (emptySpace == 0)
			uniform.offset = next;
		else
		{
			//남은 공간이 사이즈 보다 클 경우는 그냥 넣는다.
			if (emptySpace - uniform.size >= 0)
				uniform.offset = next;
			//남은 공간이 사이즈 보다 작을 경우 다음 오프셋에 넣는다.
			else
				uniform.offset = next + emptySpace;
		}
		it->second.push_back(std::move(uniform));
	}

	template<>
	inline void Shader::AddUniform<Texture>(const std::string& name, uint32_t binding, ShaderStage stage)
	{
		UniformData uniform;
		uniform.binding = binding;
		uniform.name = name;
		uniform.typeName = sh::core::reflection::GetTypeName<Texture>();
		uniform.size = 0;
		uniform.offset = 0;

		if (stage == ShaderStage::Vertex)
		{
			_samplerVertexUniforms.insert({ binding, std::move(uniform) });
		}
		else if (stage == ShaderStage::Fragment)
		{
			_samplerFragmentUniforms.insert({ binding, std::move(uniform) });
		}
	}
}