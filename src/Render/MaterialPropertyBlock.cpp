#include "MaterialPropertyBlock.h"
#include "Texture.h"

namespace sh::render
{
	SH_RENDER_API void MaterialPropertyBlock::Clear()
	{
		scalarProperties.clear();
		vecProperties.clear();
		matProperties.clear();
		intArrayProperties.clear();
		scalarArrayProperties.clear();
		vecArrayProperties.clear();
		matArrayProperties.clear();
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
	SH_RENDER_API auto MaterialPropertyBlock::GetIntArrayProperty(const std::string& name) const -> const std::vector<int>*
	{
		auto it = intArrayProperties.find(name);
		if (it == intArrayProperties.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetScalarArrayProperty(const std::string& name) const -> const std::vector<float>*
	{
		auto it = scalarArrayProperties.find(name);
		if (it == scalarArrayProperties.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetVectorArrayProperty(const std::string& name) const -> const std::vector<glm::vec4>*
	{
		auto it = vecArrayProperties.find(name);
		if (it == vecArrayProperties.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API auto MaterialPropertyBlock::GetMatrixArrayProperty(const std::string& name) const -> const std::vector<glm::mat4>*
	{
		auto it = matArrayProperties.find(name);
		if (it == matArrayProperties.end())
			return nullptr;
		return &it->second;
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
		propertiesJson["scalar"] = std::move(intJson);

		core::Json vectorJson{};
		for (auto& [name, value] : vecProperties)
			vectorJson[name] = { value.x, value.y, value.z, value.w };
		propertiesJson["vector"] = std::move(vectorJson);

		core::Json matrixJson{};
		for (auto& [name, value] : matProperties)
		{
			for (int i = 0; i < 4; ++i)
				matrixJson[name].push_back({ value[i].x, value[i].y, value[i].z, value[i].w });
		}
		propertiesJson["matrix"] = std::move(matrixJson);

		core::Json intArrayJson{};
		for (auto& [name, values] : intArrayProperties)
			intArrayJson[name] = values;
		propertiesJson["intArrays"] = std::move(intArrayJson);

		core::Json scalarArrayJson{};
		for (auto& [name, values] : scalarArrayProperties)
			scalarArrayJson[name] = values;
		propertiesJson["scalarArrays"] = std::move(scalarArrayJson);

		core::Json vectorArrayJson{};
		for (auto& [name, values] : vecArrayProperties)
		{
			for (const glm::vec4& value : values)
				vectorArrayJson[name].push_back({ value.x, value.y, value.z, value.w });
		}
		propertiesJson["vectorArrays"] = std::move(vectorArrayJson);

		core::Json matrixArrayJson{};
		for (auto& [name, values] : matArrayProperties)
		{
			for (const glm::mat4& value : values)
			{
				core::Json matJson{};
				for (int i = 0; i < 4; ++i)
					matJson.push_back({ value[i].x, value[i].y, value[i].z, value[i].w });
				matrixArrayJson[name].push_back(std::move(matJson));
			}
		}
		propertiesJson["matrixArrays"] = std::move(matrixArrayJson);

		core::Json texJson{};
		for (auto& [name, value] : textureProperties)
			texJson[name] = value->GetUUID().ToString();
		propertiesJson["textures"] = std::move(texJson);

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
		if (json.contains("matrix"))
		{
			const auto& matJson = json["matrix"];
			for (const auto& [name, value] : matJson.items())
			{
				if (!value.is_array() || value.size() != 4)
					continue;
				glm::mat4 mat{ 0.f };
				for (int i = 0; i < 4; ++i)
				{
					const core::Json& col = value[i];
					if (!col.is_array() || col.size() != 4)
						continue;
					mat[i] = { value[i][0], value[i][1], value[i][2], value[i][3] };
				}
				SetProperty(name, mat);
			}
		}
		if (json.contains("scalarArrays"))
		{
			const auto& scalarArrayJson = json["scalarArrays"];
			for (const auto& [name, value] : scalarArrayJson.items())
			{
				if (value.is_array())
				{
					std::vector<float> values = value.get<std::vector<float>>();
					SetArrayProperty(name, values.data(), values.size());
				}
			}
		}
		if (json.contains("intArrays"))
		{
			const auto& intArrayJson = json["intArrays"];
			for (const auto& [name, value] : intArrayJson.items())
			{
				if (value.is_array())
				{
					std::vector<int> values = value.get<std::vector<int>>();
					SetArrayProperty(name, values.data(), values.size());
				}
			}
		}
		if (json.contains("vectorArrays"))
		{
			const auto& vectorArrayJson = json["vectorArrays"];
			for (const auto& [name, value] : vectorArrayJson.items())
			{
				if (!value.is_array())
					continue;
				std::vector<glm::vec4> values{};
				for (const core::Json& vecJson : value)
				{
					if (!vecJson.is_array() || vecJson.size() != 4)
						continue;
					values.push_back(glm::vec4{
						vecJson[0].get<float>(),
						vecJson[1].get<float>(),
						vecJson[2].get<float>(),
						vecJson[3].get<float>()
					});
				}
				SetArrayProperty(name, values.data(), values.size());
			}
		}
		if (json.contains("matrixArrays"))
		{
			const auto& matrixArrayJson = json["matrixArrays"];
			for (const auto& [name, value] : matrixArrayJson.items())
			{
				if (!value.is_array())
					continue;
				std::vector<glm::mat4> values{};
				for (const core::Json& matJson : value)
				{
					if (!matJson.is_array() || matJson.size() != 4)
						continue;
					glm::mat4 mat{ 0.f };
					for (int i = 0; i < 4; ++i)
					{
						const core::Json& col = matJson[i];
						if (!col.is_array() || col.size() != 4)
							continue;
						mat[i] = { col[0], col[1], col[2], col[3] };
					}
					values.push_back(mat);
				}
				SetArrayProperty(name, values.data(), values.size());
			}
		}
		if (json.contains("textures"))
		{
			const auto& texJson = json["textures"];
			for (const auto& [name, value] : texJson.items())
			{
				std::string uuid = value.get<std::string>();
				auto ptr = core::SObject::GetSObjectUsingResolver(core::UUID{ uuid });
				if (!core::IsValid(ptr))
					continue;
				if (ptr->GetType() == Texture::GetStaticType())
					SetProperty(name, reinterpret_cast<const Texture*>(ptr));
			}
		}
	}
}//namespace
