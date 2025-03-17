#pragma once
#include "Export.h"
#include "ShaderPass.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

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
				const ShaderPass* passPtr;
				const UniformStructLayout* layoutPtr;
				const UniformStructLayout::UniformMember* memberPtr;
			};
			const core::reflection::TypeInfo* type;
			std::vector<Location> locations;
		};
		struct LightingPassData
		{
			std::string name;
			std::vector<std::unique_ptr<ShaderPass>> passes;
		};
	private:
		std::unordered_map<std::string, PropertyInfo> properties;
		core::SVector<LightingPassData> passes;
	private:
		void Clear();
		void AddShaderPass(std::unique_ptr<ShaderPass>&& pass);
		template<typename T>
		void AddProperty(const std::string& name);
		auto GetLightingPass(const std::string& name) -> LightingPassData*;
		auto GetLightingPass(const std::string& name) const -> const LightingPassData*;
	public:
		SH_RENDER_API Shader(ShaderCreateInfo&& shaderCreateInfo);
		SH_RENDER_API ~Shader() = default;
		SH_RENDER_API auto GetShaderPasses(const std::string& lightingPassName) const -> const std::vector<std::unique_ptr<ShaderPass>>*;
		SH_RENDER_API auto GetAllShaderPass() const -> const core::SVector<LightingPassData>&;

		SH_RENDER_API auto GetProperties() const -> const std::unordered_map<std::string, PropertyInfo>&;
		SH_RENDER_API auto GetProperty(const std::string& name) const -> const PropertyInfo*;
	};

	template<typename T>
	inline void Shader::AddProperty(const std::string& name)
	{
		PropertyInfo info{};
		info.type = &core::reflection::GetType<T>();
		for (auto& [_, passes] : passes)
		{
			for (auto& pass : passes)
			{
				auto layout = pass->HasUniformMember(name, ShaderStage::Vertex);
				if (layout == nullptr)
					layout = pass->HasUniformMember(name, ShaderStage::Fragment);
				if (layout != nullptr)
				{
					info.locations.push_back({ pass.get(), layout, layout->GetMember(name) });
					continue;
				}
			}
		}
		properties.insert_or_assign(name, std::move(info));
	}
}//namespace
