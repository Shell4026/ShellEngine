#include "Material.h"
#include "IRenderContext.h"

#include "Core/Util.h"
#include "Core/SObjectManager.h"

namespace sh::render
{
	SH_RENDER_API Material::Material() :
		shader(nullptr)
	{
		materialData = std::make_unique<MaterialData>();

		propertyBlock = core::SObject::Create<MaterialPropertyBlock>();
	}
	SH_RENDER_API Material::Material(Shader* shader) :
		shader(shader)
	{
		materialData = std::make_unique<MaterialData>();
		propertyBlock = core::SObject::Create<MaterialPropertyBlock>();
	}

	SH_RENDER_API Material::Material(Material&& other) noexcept :
		context(other.context),
		shader(other.shader),
		bPropertyDirty(other.bPropertyDirty),
		dirtyPropSet(std::move(other.dirtyPropSet)),
		materialData(std::move(other.materialData)),
		propertyBlock(other.propertyBlock)
	{
		other.context = nullptr;
		other.shader = nullptr;
		other.propertyBlock = nullptr;

		other.bPropertyDirty = false;
	}

	void Material::Clear()
	{
		if (propertyBlock)
			propertyBlock->Clear();
		materialData.reset();
	}

	void Material::SetDefaultProperties()
	{
		assert(shader != nullptr);
		for (auto& [name, propInfo] : shader->GetProperties())
		{
			if (*propInfo.type == core::reflection::GetType<int>() || *propInfo.type == core::reflection::GetType<float>())
				propertyBlock->SetProperty(name, 0.0f);
			else if (*propInfo.type == core::reflection::GetType<glm::vec2>() || *propInfo.type == core::reflection::GetType<glm::vec3>() || *propInfo.type == core::reflection::GetType<glm::vec4>())
				propertyBlock->SetProperty(name, glm::vec4{ 0.f });
			else if (*propInfo.type == core::reflection::GetType<glm::mat2>() || *propInfo.type == core::reflection::GetType<glm::mat3>() || *propInfo.type == core::reflection::GetType<glm::mat4>())
				propertyBlock->SetProperty(name, glm::mat4{ 1.f });
		}
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
	}

	SH_RENDER_API void Material::UpdateUniformBuffers()
	{
		if (!bPropertyDirty || !core::IsValid(shader))
			return;

		for (auto& [pass, uniformLayout] : dirtyPropSet)
		{
			std::vector<uint8_t> data(uniformLayout->GetSize(), 0);
			bool isSampler = false;
			for (auto& member : uniformLayout->GetMembers())
			{
				if (member.type == core::reflection::GetType<int>())
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(static_cast<int>(var.value()), data, member.offset);
					else
						SetData(0, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<float>())
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(var.value(), data, member.offset);
					else
						SetData(0.0f, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec2>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec2{ var->x, var->y }, data, member.offset);
					else
						SetData(glm::vec2{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec3>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec3{ var->x, var->y, var->z }, data, member.offset);
					else
						SetData(glm::vec3{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec4>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::vec4{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat2>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat2(*var), data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat3>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat3(*var), data, member.offset);
					else
						SetData(glm::mat3{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat4>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<Texture>())
				{
					auto var = propertyBlock->GetTextureProperty(member.name);
					if (var)
						materialData->SetTextureData(*pass, uniformLayout->type, uniformLayout->binding, var);

					isSampler = true;
				}
			}
			if (!isSampler)
				materialData->SetUniformData(*pass, uniformLayout->type, uniformLayout->binding, data.data(), core::ThreadType::Game);
		}
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

		propertyBlock->SetProperty(name, data);

		for (auto& location : propInfo->locations)
			dirtyPropSet.insert({ location.passPtr, location.layoutPtr });

		bPropertyDirty = true;
	}

	SH_RENDER_API auto Material::GetMaterialData() const -> const MaterialData&
	{
		return *materialData.get();
	}

	SH_RENDER_API void Material::OnPropertyChanged(const core::reflection::Property& prop)
	{
		bPropertyDirty = true;
	}
	SH_RENDER_API void Material::Deserialize(const core::Json& json)
	{
		Clear();
		Super::Deserialize(json);
	}
}//namespace