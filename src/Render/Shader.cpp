#include "Shader.h"
#include "ShaderCreateInfo.h"

#include <algorithm>

namespace sh::render
{
	Shader::Shader(ShaderCreateInfo&& shaderCreateInfo)
	{
		SetName(shaderCreateInfo.shaderNode.shaderName);

		for (ShaderPass* pass : shaderCreateInfo.passes)
			AddShaderPass(pass);

		for (auto& prop : shaderCreateInfo.shaderNode.properties)
		{
			const bool bLocal = prop.attribute == ShaderAST::VariableAttribute::Local ? true : false;
			switch (prop.type)
			{
			case ShaderAST::VariableType::Int:     AddProperty<      int>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Float:   AddProperty<    float>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Vec2:    AddProperty<glm::vec2>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Vec3:    AddProperty<glm::vec3>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Vec4:    AddProperty<glm::vec4>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Mat2:    AddProperty<glm::mat2>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Mat3:    AddProperty<glm::mat3>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Mat4:    AddProperty<glm::mat4>(prop.name, bLocal); break;
			case ShaderAST::VariableType::Sampler: AddProperty<  Texture>(prop.name, bLocal); break;
			}
		}

		shaderNode = std::move(shaderCreateInfo.shaderNode);

		// 코드는 보존 할 필요 없음
		for (auto& passNode : shaderNode.passes)
		{
			for (auto& stageNode : passNode.stages)
			{
				stageNode.code.clear(); 
				stageNode.functions.clear();
				stageNode.declaration.clear();
			}
		}
	}
	Shader::~Shader()
	{
	}
	SH_RENDER_API auto Shader::GetShaderPasses(const core::Name& lightingPassName) const -> const std::vector<std::reference_wrapper<ShaderPass>>*
	{
		const LightingPassData* lightingPassData = GetLightingPass(lightingPassName);
		if (lightingPassData == nullptr)
			return nullptr;
		return &lightingPassData->passes;
	}
	SH_RENDER_API auto Shader::GetAllShaderPass() const -> const std::vector<LightingPassData>&
	{
		return passDatas;
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

	SH_RENDER_API auto Shader::GetShaderAST() const -> const ShaderAST::ShaderNode&
	{
		return shaderNode;
	}
	SH_RENDER_API auto Shader::IsUsingLight() const -> bool
	{
		return bUsingLight;
	}
	SH_RENDER_API void Shader::OnDestroy()
	{
		for (ShaderPass* shaderPass : passes)
			shaderPass->Destroy();
		Super::OnDestroy();
	}
	SH_RENDER_API auto Shader::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		mainJson["shader"] = core::Json{};
		core::Json& json = mainJson["shader"];

		json["AST"] = shaderNode.Serialize();

		std::set<ShaderPass*> uniquePass;
		for (const auto& [name, passVec] : passDatas)
		{
			for (ShaderPass& pass : passVec)
				uniquePass.insert(&pass);
		}
		for (const ShaderPass* pass : uniquePass)
			json["passes"].push_back(pass->Serialize());

		return mainJson;
	}
	SH_RENDER_API void Shader::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		if (!json.contains("shader"))
			return;

		const core::Json& shaderJson = json["shader"];

		if (shaderJson.contains("AST"))
			shaderNode.Deserialize(shaderJson["AST"]);
	}

	void Shader::Clear()
	{
		properties.clear();
		passes.clear();
	}

	void Shader::AddShaderPass(ShaderPass* pass)
	{
		if (pass->IsUsingLight())
			bUsingLight = true;

		passes.push_back(pass);

		const core::Name& lightingPassName = passes.back()->GetLightingPassName();

		LightingPassData* lightingPassData = GetLightingPass(lightingPassName);
		if (lightingPassData == nullptr)
		{
			std::vector<std::reference_wrapper<ShaderPass>> v{};
			v.push_back(*passes.back());

			LightingPassData passData{ lightingPassName };
			passData.passes = std::move(v);
			passDatas.push_back(std::move(passData));
		}
		else
			lightingPassData->passes.push_back(*passes.back());
	}
	auto Shader::GetLightingPass(const core::Name& name) -> LightingPassData*
	{
		LightingPassData* passDataPtr = nullptr;
		for (auto& passData : passDatas)
		{
			if (passData.name == name)
			{
				passDataPtr = &passData;
				break;
			}
		}
		return passDataPtr;
	}
	auto Shader::GetLightingPass(const core::Name& name) const -> const LightingPassData*
	{
		const LightingPassData* passDataPtr = nullptr;
		for (auto& passData : passDatas)
		{
			if (passData.name == name)
			{
				passDataPtr = &passData;
				break;
			}
		}
		return passDataPtr;
	}

}//namepsace
