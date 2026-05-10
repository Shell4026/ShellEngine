#include "Material.h"
#include "IRenderContext.h"
#include "RenderTexture.h"

#include "Core/Util.h"
#include "Core/SContainer.hpp"
#include "Core/Logger.h"

#include <algorithm>

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
		onBufferUpdateListener(std::move(other.onBufferUpdateListener)),
		cachedConstantData(std::move(other.cachedConstantData)),
		onShaderChanged(std::move(other.onShaderChanged))
	{
		other.context = nullptr;
		other.shader = nullptr;

		other.bPropertyDirty = false;

		UpdateListener();
	}

	void Material::Clear()
	{
		cachedConstantData.clear();
		propertyBlock.Clear();
		materialData->Clear();
	}

	void Material::SetDefaultProperties()
	{
		assert(shader != nullptr);
		for (const auto& [name, propInfo] : shader->GetProperties())
		{
			if (propInfo.bLocalProperty)
				continue;

			if (propInfo.type == core::reflection::GetType<int>() || propInfo.type == core::reflection::GetType<float>())
				SetProperty(name, 0.0f);
			else if (propInfo.type == core::reflection::GetType<glm::vec2>() || propInfo.type == core::reflection::GetType<glm::vec3>() || propInfo.type == core::reflection::GetType<glm::vec4>())
				SetProperty(name, glm::vec4{ 0.f });
			else if (propInfo.type == core::reflection::GetType<glm::mat2>() || propInfo.type == core::reflection::GetType<glm::mat3>() || propInfo.type == core::reflection::GetType<glm::mat4>())
				SetProperty(name, glm::mat4{ 1.f });
			else if (propInfo.type == core::reflection::GetType<Texture>())
			{
				static core::UUID blackTexUUID{ "bbc4ef7ec45dce223297a224f8093f18" };
				auto texPtr = static_cast<Texture*>(core::SObject::GetSObjectUsingResolver(blackTexUUID));
				if (texPtr != nullptr)
					SetProperty(name, texPtr);
				else
					SH_ERROR("Can't get default texture!");
			}
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
		{
			onShaderChanged.Notify(shader);
			return;
		}
		Clear();
		SetDefaultProperties();
		for (const auto& lightingPass : shader->GetAllShaderPass())
		{
			for (const ShaderPass& pass : lightingPass.passes)
			{
				std::size_t size = 0;
				for (auto& [name, info] : pass.GetConstants())
					size += info.size;
				std::vector<uint8_t> data(size, 0);
				cachedConstantData.push_back(std::move(data));
			}
		}
		if(context != nullptr)
			Build(*context);

		onShaderChanged.Notify(shader);
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
			if (uniformLayout->usage == UniformStructLayout::Usage::Object)
				continue;

			std::vector<uint8_t> data(uniformLayout->GetSize(), 0);
			bool isSampler = false;
			for (const UniformStructLayout::UniformMember& member : uniformLayout->GetMembers())
			{
				const auto writeArrayFn = [&](const auto* values, auto convertFn)
				{
					if (values == nullptr || member.count == 0)
						return;

					const uint32_t stride = member.layoutSize / member.count;
					const uint32_t count = std::min<uint32_t>(member.count, static_cast<uint32_t>(values->size()));
					for (uint32_t i = 0; i < count; ++i)
					{
						auto converted = convertFn((*values)[i]);
						SetData(converted, data, member.offset + stride * i);
					}
				};

				if (member.typeHash == core::reflection::GetType<int>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetIntArrayProperty(member.name),
							[](int value) { return value; });
					}
					else
					{
						auto var = propertyBlock.GetScalarProperty(member.name);
						if (var.has_value())
							SetData(static_cast<int>(var.value()), data, member.offset);
						else
							SetData(0, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<float>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetScalarArrayProperty(member.name),
							[](float value) { return value; });
					}
					else
					{
						auto var = propertyBlock.GetScalarProperty(member.name);
						if (var.has_value())
							SetData(var.value(), data, member.offset);
						else
							SetData(0.0f, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec2>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetVectorArrayProperty(member.name),
							[](const glm::vec4& value) { return glm::vec2{ value.x, value.y }; });
					}
					else
					{
						auto var = propertyBlock.GetVectorProperty(member.name);
						if (var)
							SetData(glm::vec2{ var->x, var->y }, data, member.offset);
						else
							SetData(glm::vec2{ 0.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec3>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetVectorArrayProperty(member.name),
							[](const glm::vec4& value) { return glm::vec3{ value.x, value.y, value.z }; });
					}
					else
					{
						auto var = propertyBlock.GetVectorProperty(member.name);
						if (var)
							SetData(glm::vec3{ var->x, var->y, var->z }, data, member.offset);
						else
							SetData(glm::vec3{ 0.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec4>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetVectorArrayProperty(member.name),
							[](const glm::vec4& value) { return value; });
					}
					else
					{
						auto var = propertyBlock.GetVectorProperty(member.name);
						if (var)
							SetData(*var, data, member.offset);
						else
							SetData(glm::vec4{ 0.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat2>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetMatrixArrayProperty(member.name),
							[](const glm::mat4& value) { return core::Util::ConvertMat4ToMat2(value); });
					}
					else
					{
						auto var = propertyBlock.GetMatrixProperty(member.name);
						if (var)
							SetData(core::Util::ConvertMat4ToMat2(*var), data, member.offset);
						else
							SetData(glm::mat2{ 1.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat3>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetMatrixArrayProperty(member.name),
							[](const glm::mat4& value) { return core::Util::ConvertMat4ToMat3(value); });
					}
					else
					{
						auto var = propertyBlock.GetMatrixProperty(member.name);
						if (var)
							SetData(core::Util::ConvertMat4ToMat3(*var), data, member.offset);
						else
							SetData(glm::mat3{ 1.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat4>().hash)
				{
					if (member.IsArray())
					{
						writeArrayFn(propertyBlock.GetMatrixArrayProperty(member.name),
							[](const glm::mat4& value) { return value; });
					}
					else
					{
						auto var = propertyBlock.GetMatrixProperty(member.name);
						if (var)
							SetData(*var, data, member.offset);
						else
							SetData(glm::mat4{ 1.f }, data, member.offset);
					}
				}
				else if (member.typeHash == core::reflection::GetType<Texture>().hash)
				{
					auto var = propertyBlock.GetTextureProperty(member.name);
					if (core::IsValid(var))
						materialData->SetTextureData(*pass, uniformLayout->usage, uniformLayout->binding, *var);

					isSampler = true;
				}
			}
			if (!isSampler)
				materialData->SetBindingData(*pass, uniformLayout->usage, uniformLayout->binding, std::move(data));
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

		if (propInfo->type == core::reflection::GetType<glm::vec4>())
		{
			SetProperty<glm::vec4>(name, data);
		}
		else if (propInfo->type == core::reflection::GetType<glm::vec3>())
		{
			glm::vec3 dataVec3{ data };
			SetProperty(name, dataVec3);
		}
		else if (propInfo->type == core::reflection::GetType<glm::vec2>())
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
	SH_RENDER_API auto Material::GetConstantData(const ShaderPass& pass) const -> const std::vector<uint8_t>*
	{
		if (!core::IsValid(shader))
			return nullptr;
		if (cachedConstantData.empty())
			return nullptr;

		int idx = 0;
		for (auto& lightingPass : shader->GetAllShaderPass())
		{
			for (const ShaderPass& shaderPass : lightingPass.passes)
			{
				if (&shaderPass == &pass)
					return &cachedConstantData[idx];
				++idx;
			}
		}
		return nullptr;
	}
	SH_RENDER_API void Material::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("shader"))
		{
			SetShader(shader);
		}
		bPropertyDirty = true;
	}

	SH_RENDER_API auto Material::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		mainJson["MaterialPropertyBlock"] = propertyBlock.Serialize();
		for (auto& data : cachedConstantData)
			mainJson["Constant"].push_back(data);

		return mainJson;
	}
	SH_RENDER_API void Material::Deserialize(const core::Json& json)
	{
		// SetProperty에서 다른 역할도 수행하기 때문에 PropertyBlock을 직접적으로 Deserialize하지 않는다.
		Clear();
		Super::Deserialize(json);
		if (json.contains("MaterialPropertyBlock"))
		{
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
			if (propertyJson.contains("matrix"))
			{
				const auto& matJson = propertyJson["matrix"];
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
						mat[i] = { col[0], col[1], col[2], col[3] };
					}
					SetProperty(name, mat);
				}
			}
			if (propertyJson.contains("scalarArrays"))
			{
				const auto& scalarArrayJson = propertyJson["scalarArrays"];
				for (const auto& [name, value] : scalarArrayJson.items())
				{
					if (!value.is_array())
						continue;

					std::vector<float> values = value.get<std::vector<float>>();
					SetProperty(name, values);
				}
			}
			if (propertyJson.contains("intArrays"))
			{
				const auto& intArrayJson = propertyJson["intArrays"];
				for (const auto& [name, value] : intArrayJson.items())
				{
					if (!value.is_array())
						continue;

					std::vector<int> values = value.get<std::vector<int>>();
					SetProperty(name, values);
				}
			}
			if (propertyJson.contains("vectorArrays"))
			{
				const auto& vectorArrayJson = propertyJson["vectorArrays"];
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

					const Shader::PropertyInfo* propInfo = core::IsValid(shader) ? shader->GetProperty(name) : nullptr;
					if (propInfo != nullptr && propInfo->type == core::reflection::GetType<glm::vec2>())
					{
						std::vector<glm::vec2> vec2Values(values.size());
						for (std::size_t i = 0; i < values.size(); ++i)
							vec2Values[i] = glm::vec2{ values[i].x, values[i].y };
						SetProperty(name, vec2Values);
					}
					else if (propInfo != nullptr && propInfo->type == core::reflection::GetType<glm::vec3>())
					{
						std::vector<glm::vec3> vec3Values(values.size());
						for (std::size_t i = 0; i < values.size(); ++i)
							vec3Values[i] = glm::vec3{ values[i].x, values[i].y, values[i].z };
						SetProperty(name, vec3Values);
					}
					else
						SetProperty(name, values);
				}
			}
			if (propertyJson.contains("matrixArrays"))
			{
				const auto& matrixArrayJson = propertyJson["matrixArrays"];
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

					const Shader::PropertyInfo* propInfo = core::IsValid(shader) ? shader->GetProperty(name) : nullptr;
					if (propInfo != nullptr && propInfo->type == core::reflection::GetType<glm::mat2>())
					{
						std::vector<glm::mat2> mat2Values(values.size());
						for (std::size_t i = 0; i < values.size(); ++i)
							mat2Values[i] = core::Util::ConvertMat4ToMat2(values[i]);
						SetProperty(name, mat2Values);
					}
					else if (propInfo != nullptr && propInfo->type == core::reflection::GetType<glm::mat3>())
					{
						std::vector<glm::mat3> mat3Values(values.size());
						for (std::size_t i = 0; i < values.size(); ++i)
							mat3Values[i] = core::Util::ConvertMat4ToMat3(values[i]);
						SetProperty(name, mat3Values);
					}
					else
						SetProperty(name, values);
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
		if (json.contains("Constant"))
		{
			const auto& constantJson = json["Constant"];
			if (constantJson.size() == cachedConstantData.size())
			{
				int idx = 0;
				for (auto& arrJson : constantJson)
				{
					if (arrJson.size() == cachedConstantData[idx].size())
						cachedConstantData[idx] = arrJson.get<std::vector<uint8_t>>();
					++idx;
				}
			}
		}
	}
}//namespace
