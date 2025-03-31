#include "MaterialPropertyBlock.h"
#include "Texture.h"

namespace sh::render
{
	SH_RENDER_API void MaterialPropertyBlock::Clear()
	{
		scalarProperties.clear();
		vecProperties.clear();
		matProperties.clear();
		textureProperties.clear();
	}
	SH_RENDER_API void MaterialPropertyBlock::SetProperty(const std::string& name, const Texture* data)
	{
		textureProperties.insert_or_assign(name, data);
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetScalarProperty(const std::string& name) const -> std::optional<float>
	{
		auto it = scalarProperties.find(name);
		if (it == scalarProperties.end())
			return {};
		return it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetVectorProperty(const std::string& name) const -> const glm::vec4*
	{
		auto it = vecProperties.find(name);
		if (it == vecProperties.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetMatrixProperty(const std::string& name) const -> const glm::mat4*
	{
		auto it = matProperties.find(name);
		if (it == matProperties.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetTextureProperty(const std::string& name) const -> const Texture*
	{
		auto it = textureProperties.find(name);
		if (it == textureProperties.end())
			return nullptr;
		return it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetTextureProperties() const -> const std::unordered_map<std::string, const Texture*>&
	{
		return textureProperties;
	}
	SH_RENDER_API auto sh::render::MaterialPropertyBlock::Serialize() const -> core::Json
	{
		core::Json propertiesJson{};

		core::Json intJson{};
		for (auto& [name, value] : scalarProperties)
			intJson[name] = value;
		propertiesJson["scalar"] = intJson;

		core::Json vectorJson{};
		for (auto& [name, value] : vecProperties)
			vectorJson[name] = { value.x, value.y, value.z, value.w };
		propertiesJson["vector"] = vectorJson;

		// TODO
		//core::Json matrixJson{};
		//for (auto& [name, value] : matProperties)
		//	
		//propertiesJson["matrix"] = matrixJson;

		core::Json texJson{};
		for (auto& [name, value] : textureProperties)
			texJson[name] = value->GetUUID().ToString();
		propertiesJson["textures"] = texJson;

		return propertiesJson;
	}
	SH_RENDER_API void sh::render::MaterialPropertyBlock::Deserialize(const core::Json& json)
	{
		if (json.contains("scalar"))
		{
			const auto& intJson = json["scalar"];
			for (const auto& [name, value] : intJson.items())
			{
				SetProperty(name, value.get<float>());
			}
		}
		if (json.contains("vector"))
		{
			const auto& vectorJson = json["vector"];
			for (const auto& [name, value] : vectorJson.items())
			{
				if (value.is_array() && value.size() == 4)
				{
					glm::vec4 vecValue(
						value[0].get<float>(),
						value[1].get<float>(),
						value[2].get<float>(),
						value[3].get<float>()
					);
					SetProperty(name, vecValue);
				}
			}
		}
		if (json.contains("textures"))
		{
			const auto& texJson = json["textures"];
			for (const auto& [name, value] : texJson.items())
			{
				std::string uuid = value.get<std::string>();
				auto ptr = core::SObjectManager::GetInstance()->GetSObject(uuid);
				if (!core::IsValid(ptr))
					continue;
				if (ptr->GetType() == Texture::GetStaticType())
					SetProperty(name, reinterpret_cast<const Texture*>(ptr));
			}
		}
	}
}//namespace