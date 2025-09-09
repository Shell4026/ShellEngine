#include "Material.h"
#include "IRenderContext.h"
#include "RenderTexture.h"

#include "Core/Util.h"
#include "Core/SObjectManager.h"
#include "Core/SContainer.hpp"
#include "Core/AssetResolver.h"

namespace sh::render
{
	SH_RENDER_API Material::Material() :
		shader(nullptr)
	{
		materialData = std::make_unique<MaterialData>();

		UpdateListener();
	}
	SH_RENDER_API Material::Material(Shader* shader) :
		shader(shader)
	{
		materialData = std::make_unique<MaterialData>();

		UpdateListener();
		SetDefaultProperties();
	}
	Material::Material(const Material& other) :
		core::SObject(other),
		context(other.context),
		shader(other.shader),
		propertyBlock(other.propertyBlock)
	{
		materialData = std::make_unique<MaterialData>();

		Deserialize(Serialize());

		Build(*context);
	}
	SH_RENDER_API Material::Material(Material&& other) noexcept :
		core::SObject(std::move(other)),
		context(other.context),
		shader(other.shader),
		bPropertyDirty(other.bPropertyDirty),
		dirtyProps(std::move(other.dirtyProps)),
		materialData(std::move(other.materialData)),
		propertyBlock(std::move(other.propertyBlock)),
		onBufferUpdateListener(std::move(other.onBufferUpdateListener))
	{
		other.context = nullptr;
		other.shader = nullptr;

		other.bPropertyDirty = false;

		UpdateListener();
	}

	void Material::Clear()
	{
		propertyBlock.Clear();
		materialData->Clear();
	}

	void Material::SetDefaultProperties()
	{
		assert(shader != nullptr);
		for (auto& [name, propInfo] : shader->GetProperties())
		{
			if (*propInfo.type == core::reflection::GetType<int>() || *propInfo.type == core::reflection::GetType<float>())
				propertyBlock.SetProperty(name, 0.0f);
			else if (*propInfo.type == core::reflection::GetType<glm::vec2>() || *propInfo.type == core::reflection::GetType<glm::vec3>() || *propInfo.type == core::reflection::GetType<glm::vec4>())
				propertyBlock.SetProperty(name, glm::vec4{ 0.f });
			else if (*propInfo.type == core::reflection::GetType<glm::mat2>() || *propInfo.type == core::reflection::GetType<glm::mat3>() || *propInfo.type == core::reflection::GetType<glm::mat4>())
				propertyBlock.SetProperty(name, glm::mat4{ 1.f });
		}
	}
	void Material::UpdateListener()
	{
		onBufferUpdateListener.SetCallback([&](const Texture* updated)
			{
				for (auto& [name, tex] : propertyBlock.GetTextureProperties())
				{
					if (tex != updated)
						continue;

					SetProperty(name, updated);
					UpdateUniformBuffers();
				}
			}
		);
	}

	SH_RENDER_API void Material::SetShader(Shader* shader)
	{
		this->shader = shader;
		if (!core::IsValid(this->shader))
			return;
		if (context == nullptr)
			return;

		Clear();

		SetDefaultProperties();

		Build(*context);
	}
	
	SH_RENDER_API auto Material::GetShader() const -> Shader*
	{
		return shader;
	}

	SH_RENDER_API void Material::Build(const IRenderContext& context)
	{
		this->context = &context;

		if (!core::IsValid(shader))
			return;
		
		materialData->Create(context, *shader);

		UpdateUniformBuffers();
	}

	SH_RENDER_API void Material::UpdateUniformBuffers()
	{
		if (!bPropertyDirty || !core::IsValid(shader))
			return;

		for (auto& [pass, uniformLayout] : dirtyProps)
		{
			std::vector<uint8_t> data(uniformLayout->GetSize(), 0);
			bool isSampler = false;
			for (auto& member : uniformLayout->GetMembers())
			{
				if (member.typeHash == core::reflection::GetType<int>().hash)
				{
					auto var = propertyBlock.GetScalarProperty(member.name);
					if (var.has_value())
						SetData(static_cast<int>(var.value()), data, member.offset);
					else
						SetData(0, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<float>().hash)
				{
					auto var = propertyBlock.GetScalarProperty(member.name);
					if (var.has_value())
						SetData(var.value(), data, member.offset);
					else
						SetData(0.0f, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec2>().hash)
				{
					auto var = propertyBlock.GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec2{ var->x, var->y }, data, member.offset);
					else
						SetData(glm::vec2{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec3>().hash)
				{
					auto var = propertyBlock.GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec3{ var->x, var->y, var->z }, data, member.offset);
					else
						SetData(glm::vec3{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec4>().hash)
				{
					auto var = propertyBlock.GetVectorProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::vec4{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat2>().hash)
				{
					auto var = propertyBlock.GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat2(*var), data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat3>().hash)
				{
					auto var = propertyBlock.GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat3(*var), data, member.offset);
					else
						SetData(glm::mat3{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat4>().hash)
				{
					auto var = propertyBlock.GetMatrixProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<Texture>().hash)
				{
					auto var = propertyBlock.GetTextureProperty(member.name);
					if (var)
						materialData->SetTextureData(*pass, uniformLayout->type, uniformLayout->binding, var);

					isSampler = true;
				}
			}
			if (!isSampler)
				materialData->SetUniformData(*pass, uniformLayout->type, uniformLayout->binding, data.data());
		}
		dirtyProps.clear();
		bPropertyDirty = false;
	}

	SH_RENDER_API void Material::SetProperty(const std::string& name, const Texture* data)
	{
		if (!core::IsValid(shader))
			return;

		const Shader::PropertyInfo* propInfo = shader->GetProperty(name);
		assert(propInfo != nullptr);
		if (propInfo == nullptr)
			return;

		if (auto last = GetProperty<const Texture*>(name); last.has_value())
		{
			if (last.value() != data && last.value() != nullptr)
				last.value()->onBufferUpdate.UnRegister(onBufferUpdateListener);
		}
		if (core::IsValid(data))
			data->onBufferUpdate.Register(onBufferUpdateListener);

		propertyBlock.SetProperty(name, data);

		for (auto& location : propInfo->locations)
		{
			auto it = std::find(dirtyProps.begin(), dirtyProps.end(), std::pair{ location.passPtr.Get(), location.layoutPtr});
			if (it == dirtyProps.end())
				dirtyProps.push_back({ location.passPtr.Get(), location.layoutPtr});
		}

		bPropertyDirty = true;
	}
	SH_RENDER_API void Material::SetProperty(const std::string& name, const glm::vec4& data)
	{
		if (!core::IsValid(shader))
			return;
		const Shader::PropertyInfo* propInfo = shader->GetProperty(name);
		if (propInfo == nullptr)
			return;

		if (*propInfo->type == core::reflection::GetType<glm::vec4>())
		{
			SetProperty<glm::vec4>(name, data);
		}
		else if (*propInfo->type == core::reflection::GetType<glm::vec3>())
		{
			glm::vec3 dataVec3{ data };
			SetProperty(name, dataVec3);
		}
		else if (*propInfo->type == core::reflection::GetType<glm::vec2>())
		{
			glm::vec2 dataVec2{ data };
			SetProperty(name, dataVec2);
		}
	}
	SH_RENDER_API void Material::SetProperty(const std::string& name, Texture* data)
	{
		SetProperty(name, static_cast<const Texture*>(data));
	}
	SH_RENDER_API void Material::SetProperty(const std::string& name, const RenderTexture* data)
	{
		SetProperty(name, static_cast<const Texture*>(data));
	}
	SH_RENDER_API void Material::SetProperty(const std::string& name, RenderTexture* data)
	{
		SetProperty(name, static_cast<const Texture*>(data));
	}

	SH_RENDER_API auto Material::GetMaterialData() const -> const MaterialData&
	{
		return *materialData.get();
	}

	SH_RENDER_API void Material::OnPropertyChanged(const core::reflection::Property& prop)
	{
		bPropertyDirty = true;
	}

	SH_RENDER_API auto Material::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		mainJson["MaterialPropertyBlock"] = propertyBlock.Serialize();
		return mainJson;
	}
	SH_RENDER_API void Material::Deserialize(const core::Json& json)
	{
		// SetProperty에서 다른 역할도 수행하기 때문에 PropertyBlock을 직접적으로 Deserialize하지 않는다.
		Clear();
		Super::Deserialize(json);
		const auto& propertyJson = json["MaterialPropertyBlock"];
		if (propertyJson.contains("scalar"))
		{
			const auto& intJson = propertyJson["scalar"];
			for (const auto& [name, value] : intJson.items())
			{
				SetProperty(name, value.get<float>());
			}
		}
		if (propertyJson.contains("vector"))
		{
			const auto& vectorJson = propertyJson["vector"];
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
		if (propertyJson.contains("textures"))
		{
			const auto& texJson = propertyJson["textures"];
			for (const auto& [name, value] : texJson.items())
			{
				const core::UUID uuid{ value.get<std::string>() };
				auto ptr = core::SObject::GetSObjectUsingResolver(uuid);
				if (!core::IsValid(ptr))
					continue;
				if (ptr->GetType() == Texture::GetStaticType())
					SetProperty(name, reinterpret_cast<const Texture*>(ptr));
			}
		}
	}
}//namespace