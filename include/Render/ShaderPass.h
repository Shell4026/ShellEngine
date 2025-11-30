#pragma once
#include "Export.h"
#include "Texture.h"
#include "StencilState.h"
#include "UniformStructLayout.h"
#include "ShaderEnum.h"
#include "ShaderAST.h"

#include "Core/SObject.h"
#include "Core/Name.h"
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
	class ShaderPass : public core::SObject, public core::INonCopyable
	{
		SCLASS(ShaderPass)
	public:
		struct AttributeData
		{
			uint32_t idx;
			std::size_t typeHash;
			std::size_t size;
			std::string name;
		};
		struct ShaderCode
		{
			std::vector<uint8_t> vert;
			std::vector<uint8_t> frag;
		};
		struct ConstantInfo
		{
			uint32_t offset;
			uint32_t size;
			uint32_t constantID;
		};
	public:
		SH_RENDER_API virtual ~ShaderPass();
		SH_RENDER_API void operator=(ShaderPass&& other) noexcept;

		SH_RENDER_API virtual void Clear() = 0;
		SH_RENDER_API virtual void Build() = 0;

		SH_RENDER_API auto HasUniformMember(const std::string& name, ShaderStage stage) const -> const UniformStructLayout*;

		SH_RENDER_API auto GetStencilState() const -> const StencilState&;
		SH_RENDER_API auto GetCullMode() const->CullMode;
		SH_RENDER_API auto GetZWrite() const -> bool;
		/// @brief RGBA전부 쓰기면 0b1111, R만 쓰기면 0b0001, G만 쓴다면 0b0010, B만 쓴다면 0b0100, A만 쓴다면 0b1000
		/// @return 컬러 마스크 값
		SH_RENDER_API auto GetColorMask() const -> uint8_t;
		SH_RENDER_API auto GetLightingPassName() const -> const core::Name&;
		SH_RENDER_API auto GetId() const -> int;
		SH_RENDER_API auto GetShaderType() const->ShaderType;
		SH_RENDER_API auto GetAttributes() const -> const std::vector<AttributeData>&;
		SH_RENDER_API auto GetVertexUniforms() const -> const std::vector<UniformStructLayout>&;
		SH_RENDER_API auto GetFragmentUniforms() const -> const std::vector<UniformStructLayout>&;
		SH_RENDER_API auto GetSamplerUniforms() const -> const std::vector<UniformStructLayout>&;
		SH_RENDER_API auto HasConstantUniform() const -> bool;
		SH_RENDER_API auto IsUsingLight() const -> bool;
		SH_RENDER_API auto GetConstantsInfo(const std::string& name) const -> const ConstantInfo*;
		SH_RENDER_API auto GetConstantSize() const -> std::size_t;

		SH_RENDER_API bool HasAttribute(const std::string& name) const;
		SH_RENDER_API auto GetAttribute(const std::string& name) const -> std::optional<AttributeData>;

		SH_RENDER_API void StoreShaderCode(ShaderCode&& shaderCode);

		/// @brief 셰이더 바이너리 데이터를 직렬화 한다.
		/// @return 직렬화 된 json
		SH_RENDER_API auto Serialize() const -> core::Json override;
		/// @brief 셰이더 바이너리 데이터를 역직렬화 한다.
		/// @param json 직렬화 된 json
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	protected:
		ShaderPass(const ShaderAST::PassNode& passNode, ShaderType type);
		ShaderPass(const ShaderPass& other);
		ShaderPass(ShaderPass&& other) noexcept;
	private:
		void AddUniformLayout(ShaderStage stage, const UniformStructLayout& layout);
		void AddUniformLayout(ShaderStage stage, UniformStructLayout&& layout);
		void SetStencilState(StencilState stencilState);
		void FillAttributes(const render::ShaderAST::PassNode& passNode);

		template<typename T>
		bool AddAttribute(const std::string& name, uint32_t loc);

		auto IsSamplerLayout(const UniformStructLayout& layout) const -> bool;
	protected:
		std::vector<AttributeData> attrs;
		std::unordered_map<std::string, uint32_t> attridx;

		std::vector<UniformStructLayout> vertexUniforms;
		std::vector<UniformStructLayout> fragmentUniforms;
		std::vector<UniformStructLayout> samplerUniforms;
		std::unordered_map<std::string, ConstantInfo> constantNameMap;

		ShaderCode shaderCode;

		ShaderType type;
	private:
		StencilState stencilState{};
		CullMode cull = CullMode::Back;
		core::Name lightingPassName;
		uint8_t colorMask = 7; //0b111
		std::size_t constantSize = 0;
		bool bZWrite = true;
		bool bHasConstant = false;
		bool bUseLighting = false;
	};

	template<typename T>
	inline bool ShaderPass::AddAttribute(const std::string& name, uint32_t loc)
	{
		auto it = attridx.find(name);
		if (it != attridx.end())
			return false;

		if (attrs.size() < loc + 1)
		{
			attrs.resize(loc + 1);
		}
		AttributeData  attr;
		attr.idx = loc;
		attr.name = name;
		attr.typeHash = sh::core::reflection::GetType<T>().hash;
		attr.size = sizeof(T);

		attrs[loc] = attr;
		attridx.insert({ name, loc });

		return true;
	}
}