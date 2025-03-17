#include "Shader.h"
#include "ShaderCreateInfo.h"

#include <algorithm>

namespace sh::render
{
	void Shader::Clear()
	{
		properties.clear();
		passes.clear();
	}

	void Shader::AddShaderPass(std::unique_ptr<ShaderPass>&& pass)
	{
		auto& lightingPassName = pass->GetLightingPassName();

		LightingPassData* lightingPassData = GetLightingPass(lightingPassName);
		if (lightingPassData == nullptr)
		{
			std::vector<std::unique_ptr<ShaderPass>> v{};
			v.push_back(std::move(pass));
			passes.push_back(LightingPassData{ lightingPassName, std::move(v) });
		}
		else
			lightingPassData->passes.push_back(std::move(pass));
	}
	auto Shader::GetLightingPass(const std::string& name) -> LightingPassData*
	{
		auto it = std::find_if(passes.begin(), passes.end(), [&](const LightingPassData& lightingPassData)
		{
			return lightingPassData.name == name;
		});
		if (it == passes.end())
			return nullptr;
		return &(*it);
	}
	auto Shader::GetLightingPass(const std::string& name) const -> const LightingPassData*
	{
		auto it = std::find_if(passes.begin(), passes.end(), [&](const LightingPassData& lightingPassData)
		{
			return lightingPassData.name == name;
		});
		if (it == passes.end())
			return nullptr;
		return &(*it);
	}

	Shader::Shader(ShaderCreateInfo&& shaderCreateInfo)
	{
		SetName(shaderCreateInfo.GetShaderNode().shaderName);

		for (auto& pass : shaderCreateInfo.GetShaderPasses())
			AddShaderPass(std::move(pass));

		for (auto& prop : shaderCreateInfo.GetShaderNode().properties)
		{
			switch (prop.type)
			{
			case ShaderAST::VariableType::Int:     AddProperty<      int>(prop.name); break;
			case ShaderAST::VariableType::Float:   AddProperty<    float>(prop.name); break;
			case ShaderAST::VariableType::Vec2:    AddProperty<glm::vec2>(prop.name); break;
			case ShaderAST::VariableType::Vec3:    AddProperty<glm::vec3>(prop.name); break;
			case ShaderAST::VariableType::Vec4:    AddProperty<glm::vec4>(prop.name); break;
			case ShaderAST::VariableType::Mat2:    AddProperty<glm::mat2>(prop.name); break;
			case ShaderAST::VariableType::Mat3:    AddProperty<glm::mat3>(prop.name); break;
			case ShaderAST::VariableType::Mat4:    AddProperty<glm::mat4>(prop.name); break;
			case ShaderAST::VariableType::Sampler: AddProperty<  Texture>(prop.name); break;
			}
		}
	}
	SH_RENDER_API auto Shader::GetShaderPasses(const std::string& lightingPassName) const -> const std::vector<std::unique_ptr<ShaderPass>>*
	{
		const LightingPassData* lightingPassData = GetLightingPass(lightingPassName);
		if (lightingPassData == nullptr)
			return nullptr;
		return &lightingPassData->passes;
	}
	SH_RENDER_API auto Shader::GetAllShaderPass() const -> const core::SVector<LightingPassData>&
	{
		return passes;
	}
	SH_RENDER_API auto Shader::GetProperties() const -> const std::unordered_map<std::string, PropertyInfo>&
	{
		return properties;
	}
	SH_RENDER_API auto Shader::GetProperty(const std::string& name) const -> const PropertyInfo*
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return nullptr;
		return &it->second;
	}
}//namepsace
