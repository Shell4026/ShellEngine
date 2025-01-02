#pragma once

#include "Export.h"
#include "Texture.h"
#include "StencilState.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"
#include "Core/SContainer.hpp"
#include "Core/Util.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <optional>

namespace sh::render
{
	class Shader : public sh::core::SObject, sh::core::INonCopyable
	{
		SCLASS(Shader)
	private:
		StencilState stencilState{};
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
		enum class UniformType
		{
			Object, // 객체마다 고유
			Material // 메테리얼간 공유
		};

		struct Data
		{
			uint32_t idx;
			std::string_view typeName;
			std::string name;
			std::size_t size;
		};
		struct UniformData
		{
			uint32_t binding;
			std::string name;
			core::reflection::TypeInfo& type;
			std::size_t offset;
			std::size_t size;
			std::size_t idx = 0; // 배열 인덱스
			bool bArray = false;
		};
		struct UniformBlock
		{
			UniformType type;
			uint32_t binding;
			uint32_t align;
			core::SVector<UniformData> data;

			SH_RENDER_API UniformBlock() :
				type(UniformType::Material), binding(0), align(4), data()
			{}
			SH_RENDER_API UniformBlock(const UniformBlock& other) :
				type(other.type), binding(other.binding), align(other.align), data(other.data)
			{}
			SH_RENDER_API UniformBlock(UniformBlock&& other) noexcept :
				type(other.type), binding(other.binding), align(other.align), data(std::move(other.data))
			{}
		};
		using UniformMap = std::map<uint32_t, std::vector<UniformData>>;
	protected:
		std::vector<Data> attrs;
		core::SHashMap<std::string, uint32_t> attridx;
		core::SHashMap<std::string, uint32_t> uniformBindings;

		core::SVector<UniformBlock> vertexUniforms;
		core::SVector<UniformBlock> fragmentUniforms;
		core::SVector<UniformData> samplerUniforms;

		ShaderType type;
		int id;
	protected:
		Shader(int id, ShaderType type);
		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
	public:
		SH_RENDER_API virtual ~Shader() = default;
		SH_RENDER_API void operator=(Shader&& other) noexcept;
		SH_RENDER_API auto operator==(const Shader& other) -> bool;

		SH_RENDER_API virtual void Clean() = 0;
		SH_RENDER_API virtual void Build() = 0;

		SH_RENDER_API void SetStencilState(StencilState stencilState);
		SH_RENDER_API auto GetStencilState() const -> const StencilState&;
		SH_RENDER_API auto GetShaderType() const->ShaderType;
		SH_RENDER_API auto GetAttributes() const -> const std::vector<Data>&;
		SH_RENDER_API auto GetVertexUniforms() const -> const std::vector<UniformBlock>&;
		SH_RENDER_API auto GetFragmentUniforms() const -> const std::vector<UniformBlock>&;
		SH_RENDER_API auto GetSamplerUniforms() const -> const std::vector<UniformData>&;
		SH_RENDER_API auto GetUniformBinding(std::string_view name) const -> std::optional<uint32_t>;

		template<typename T>
		bool AddAttribute(const std::string& name, uint32_t loc);
		SH_RENDER_API bool HasAttribute(const std::string& name) const;
		SH_RENDER_API auto GetAttribute(const std::string& name) const -> std::optional<Data>;

		template<typename T>
		void AddUniform(const std::string& name, UniformType type, uint32_t binding, ShaderStage stage);

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
	/// @brief 유니폼을 추가하는 함수. 반드시 셰이더의 유니폼과 같은 순서로 호출하여 넣을 것.
	/// @tparam T 타입
	/// @param name 이름
	/// @param type 유니폼 타입
	/// @param binding 바인딩 번호
	/// @param stage 셰이더 스테이지
	template<typename T>
	inline void Shader::AddUniform(const std::string& name, UniformType type, uint32_t binding, ShaderStage stage)
	{
		std::size_t align = (sizeof(T) > 4) ? ((sizeof(T) == 8) ? 8 : 16) : 4; // 4바이트, 8바이트 16바이트 정렬로만 이루어짐
		core::reflection::TypeInfo* typeinfo = &core::reflection::GetType<T>();
		std::size_t size = sizeof(T);
		// 배열인 경우
		if constexpr (core::reflection::IsArray<T>::value)
		{
			typeinfo = &core::reflection::GetType<typename core::reflection::GetContainerLastType<T>::type>();
			align = 16; // 배열의 경우 무조건 16바이트 정렬
			size = sizeof(typename core::reflection::GetContainerLastType<T>::type);
		}
		
		UniformData uniform{ binding, name, *typeinfo, 0, size };

		auto uniformVec = &vertexUniforms;
		if (stage == ShaderStage::Fragment)
			uniformVec = &fragmentUniforms;

		// 이미 같은 블록이 있는 경우
		for (auto& block : *uniformVec)
		{
			if (block.binding == binding && block.type == type)
			{
				uniformBindings.insert({ name, binding });

				if (align > block.align)
					block.align = align;

				std::size_t size = block.data.back().offset + block.data.back().size;
				uniform.offset = core::Util::AlignTo(size, block.align);
				
				// 배열인 경우
				if constexpr (core::reflection::IsArray<T>::value)
				{
					std::size_t count = sizeof(T) / sizeof(typename core::reflection::GetContainerLastType<T>::type);
					for (std::size_t i = 0; i < count; ++i)
					{
						uniform.bArray = true;
						uniform.idx = i;
						block.data.push_back(uniform);
						uniform.offset += 16;
					}
				}
				else
					block.data.push_back(uniform);

				return;
			}
		}
		// 없는 경우 새로 만듦
		uniformBindings.insert({ name, binding });

		UniformBlock block{};
		block.type = type;
		block.binding = binding;
		block.align = align;

		// 배열인 경우
		if constexpr (core::reflection::IsArray<T>::value)
		{
			std::size_t count = sizeof(T) / sizeof(typename core::reflection::GetContainerLastType<T>::type);
			for(std::size_t i = 0; i < count; ++i)
			{
				uniform.bArray = true;
				uniform.idx = i;
				block.data.push_back(uniform);
				uniform.offset += 16;
			}
		}
		else
			block.data.push_back(uniform);

		uniformVec->push_back(block);
	}

	template<>
	inline void Shader::AddUniform<Texture>(const std::string& name, UniformType type, uint32_t binding, ShaderStage stage)
	{
		UniformData uniform{ binding, name, core::reflection::GetType<Texture>(), 0, 0 };

		for (auto& uniform : samplerUniforms)
		{
			if (uniform.name == name)
				return;
		}

		uniformBindings.insert({ name, binding });
		samplerUniforms.push_back(std::move(uniform));
	}
}