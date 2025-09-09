#pragma once
#include "Export.h"
#include "ShaderPass.h"
#include "ShaderAST.h"

#include "Core/SObject.h"
#include "Core/Name.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
namespace sh::render
{
	class ShaderCreateInfo;
	class Shader : public core::SObject
	{
		SCLASS(Shader)
	public:
		struct PropertyInfo
		{
			struct Location
			{
				core::SObjWeakPtr<const ShaderPass> passPtr;
				const UniformStructLayout* layoutPtr;
				const UniformStructLayout::UniformMember* memberPtr;
			};
			const core::reflection::TypeInfo* type;
			std::vector<Location> locations;
		};
		struct LightingPassData
		{
			core::Name name;
			core::SVector<ShaderPass*> passes;
		};
	private:
		std::unordered_map<std::string, PropertyInfo> properties;
		std::vector<LightingPassData> passes;

		ShaderAST::ShaderNode shaderNode;
	private:
		void Clear();
		void AddShaderPass(ShaderPass* pass);
		template<typename T>
		void AddProperty(const std::string& name);
		auto GetLightingPass(const core::Name& name) -> LightingPassData*;
		auto GetLightingPass(const core::Name& name) const -> const LightingPassData*;
	public:
		SH_RENDER_API Shader(ShaderCreateInfo&& shaderCreateInfo);
		SH_RENDER_API ~Shader();
		SH_RENDER_API auto GetShaderPasses(const core::Name& lightingPassName) const -> const std::vector<ShaderPass*>*;
		SH_RENDER_API auto GetAllShaderPass() const -> const std::vector<LightingPassData>&;

		SH_RENDER_API auto GetProperties() const -> const std::unordered_map<std::string, PropertyInfo>&;
		SH_RENDER_API auto GetProperty(const std::string& name) const -> const PropertyInfo*;

		SH_RENDER_API auto GetShaderAST() const -> const ShaderAST::ShaderNode&;

		SH_RENDER_API void OnDestroy() override;
		/// @brief AST와 셰이더 패스를 직렬화 한다.
		/// @return 직렬화 된 json
		SH_RENDER_API auto Serialize() const -> core::Json override;
		/// @brief AST만 역직렬화 시킨다.
		/// @param json 직렬화 된 json
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};

	template<typename T>
	inline void Shader::AddProperty(const std::string& name)
	{
		PropertyInfo info{};
		info.type = &core::reflection::GetType<T>();
		for (auto& [_, passes] : passes)
		{
			for (ShaderPass* pass : passes)
			{
				auto layout = pass->HasUniformMember(name, ShaderStage::Vertex);
				if (layout == nullptr)
					layout = pass->HasUniformMember(name, ShaderStage::Fragment);
				if (layout != nullptr)
				{
					info.locations.push_back({ pass, layout, layout->GetMember(name) });
					continue;
				}
			}
		}
		properties.insert_or_assign(name, std::move(info));
	}
}//namespace
