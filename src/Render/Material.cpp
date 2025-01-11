#include "Material.h"

#include "BufferFactory.h"
#include "IBuffer.h"
#include "IUniformBuffer.h"

#include "Core/Util.h"
#include "Core/SObjectManager.h"
#include "Core/ThreadSyncManager.h"

namespace sh::render
{
	SH_RENDER_API Material::Material() :
		shader(nullptr),
		bDirty(false), bBufferDirty(false), bBufferSync(false)
	{
	}
	SH_RENDER_API Material::Material(Shader* shader) :
		shader(shader),
		bDirty(false), bBufferDirty(false), bBufferSync(false)
	{
	}

	SH_RENDER_API Material::~Material()
	{
		Clean();
	}

	SH_RENDER_API Material::Material(Material&& other) noexcept :
		context(other.context),
		shader(other.shader),
		bDirty(other.bDirty), bBufferDirty(other.bBufferDirty), bBufferSync(other.bBufferSync),
		perPassData(std::move(other.perPassData)),
		floats(std::move(other.floats)),
		ints(std::move(other.ints)),
		mats(std::move(other.mats)),
		vectors(std::move(other.vectors)),
		floatArr(std::move(other.floatArr)),
		vectorArrs(std::move(other.vectorArrs)),
		textures(std::move(other.textures))
	{
		other.context = nullptr;
		other.shader = nullptr;

		other.bDirty = false;
		other.bBufferDirty = false;
		other.bBufferSync = false;
	}

	void Material::Clean()
	{
		for (int thr = 0; thr < core::SYNC_THREAD_NUM; ++thr)
			perPassData[thr].clear();

		floats.clear();
		ints.clear();
		vectors.clear();
		mats.clear();
		floatArr.reset();
		vectorArrs.clear();
		textures.clear();
	}

	SH_RENDER_API void Material::SetShader(Shader* shader)
	{
		this->shader = shader;
		if (!core::IsValid(this->shader))
			return;
		if (context == nullptr)
			return;

		Clean();

		SetDefaultProperties();

		Build(*context);
	}
	
	SH_RENDER_API auto Material::GetShader() const -> Shader*
	{
		return shader;
	}

	SH_RENDER_API void Material::Build(const IRenderContext& context)
	{
		if (!core::IsValid(shader))
			return;

		this->context = &context;

		for (auto& shaderPass : shader->GetPasses())
		{
			for (std::size_t thr = 0; thr < core::SYNC_THREAD_NUM; ++thr)
				perPassData[thr].push_back(PassData{});

			// 버텍스 유니폼
			for (auto& uniformBlock : shaderPass->GetVertexUniforms())
			{
				if (uniformBlock.type == ShaderPass::UniformType::Object)
					continue;

				std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
				std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

				for (std::size_t thr = 0; thr < core::SYNC_THREAD_NUM; ++thr)
					perPassData[thr].back().vertShaderData.insert({uniformBlock.binding, BufferFactory::Create(context, size)});
			}
			// 픽셀 유니폼
			for (auto& uniformBlock : shaderPass->GetFragmentUniforms())
			{
				if (uniformBlock.type == ShaderPass::UniformType::Object)
					continue;

				std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
				std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

				for (std::size_t thr = 0; thr < core::SYNC_THREAD_NUM; ++thr)
					perPassData[thr].back().fragShaderData.insert({ uniformBlock.binding, BufferFactory::Create(context, size) });
			}
			for (std::size_t thr = 0; thr < core::SYNC_THREAD_NUM; ++thr)
				perPassData[thr].back().uniformBuffer = BufferFactory::CreateUniformBuffer(context, *shaderPass, ShaderPass::UniformType::Material);
		}
		bBufferDirty = true;
	}

	void Material::SetUniformData(std::size_t passIdx, uint32_t binding, const void* data, ShaderPass::ShaderStage stage)
	{
		auto& uniformData = perPassData[core::ThreadType::Game][passIdx];
		if (stage == ShaderPass::ShaderStage::Vertex)
		{
			auto it = uniformData.vertShaderData.find(binding);
			if (it == uniformData.vertShaderData.end())
				return;

			it->second->SetData(data);
			uniformData.uniformBuffer->Update(binding, *it->second);
		}
		else if (stage == ShaderPass::ShaderStage::Fragment)
		{
			auto it = uniformData.fragShaderData.find(binding);
			if (it == uniformData.fragShaderData.end())
				return;

			it->second->SetData(data);
			uniformData.uniformBuffer->Update(binding, *it->second);
		}
		bBufferSync = true;
		SetDirty();
	}
	void Material::SetTextureData(std::size_t passIdx, uint32_t binding, Texture* tex)
	{
		if (!core::IsValid(tex) || context == nullptr)
			return;

		ShaderPass* shaderPass = shader->GetPass(passIdx);
		bool find = false;
		for (auto& uniform : shaderPass->GetSamplerUniforms())
		{
			if (uniform.binding == binding)
			{
				find = true;
				break;
			}
		}
		if (!find)
			return;

		perPassData[core::ThreadType::Game][passIdx].uniformBuffer->Update(binding, *tex);

		bBufferSync = true;
		SetDirty();
	}

	void Material::FillData(const ShaderPass::UniformBlock& uniformBlock, std::vector<uint8_t>& dst, uint32_t binding)
	{
		for (auto& data : uniformBlock.data)
		{
			if (data.type == core::reflection::GetType<int>())
			{
				auto it = ints.find(data.name);
				if (it == ints.end())
				{
					int value = 0;
					SetData(value, dst, data.offset);
				}
				else
					SetData(it->second, dst, data.offset);
			}
			else if (data.type == core::reflection::GetType<float>())
			{
				if (data.size == sizeof(float))
				{
					auto it = floats.find(data.name);
					if (it == floats.end())
					{
						float value = 0.f;
						SetData(value, dst, data.offset);
					}
					else
						SetData(it->second, dst, data.offset);
				}
				else
				{
					std::size_t count = data.size / sizeof(float);
					std::memset(dst.data() + data.offset, 0, data.size);
					if (floatArr)
					{
						auto it = floatArr->find(data.name);
						if (it != floatArr->end())
						{
							for (std::size_t i = 0; i < count; ++i)
							{
								if (i >= it->second.size())
									break;
								SetData(it->second[i], dst, data.offset + sizeof(float) * i);
							}
						}
					}
				}
			}
			else if (data.type == core::reflection::GetType<glm::vec2>())
			{
				auto it = vectors.find(data.name);
				if (it == vectors.end())
				{
					SetData(glm::vec2{ 0.f }, dst, data.offset);
				}
				else
				{

					SetData(glm::vec2{ it->second }, dst, data.offset);
				}
			}
			else if (data.type == core::reflection::GetType<glm::vec3>())
			{
				if (data.size == sizeof(glm::vec3))
				{
					auto it = vectors.find(data.name);
					if (it == vectors.end())
						SetData(glm::vec3{ 0.f }, dst, data.offset);
					else
						SetData(glm::vec3{ it->second }, dst, data.offset);
				}
				else
				{
					std::memset(dst.data() + data.offset, 0, data.size);
					std::size_t count = data.size / sizeof(glm::vec3);
					auto it = vectorArrs.find(data.name);
					if (it != vectorArrs.end())
					{
						for (std::size_t i = 0; i < count; ++i)
						{
							if (i >= it->second.size())
								break;
							SetData(it->second[i], dst, data.offset + sizeof(glm::vec4) * i); // vec3 패딩
						}
					}
				}
			}
			else if (data.type == core::reflection::GetType<glm::vec4>())
			{
				if (!data.bArray)
				{
					auto it = vectors.find(data.name);
					if (it == vectors.end())
						SetData(glm::vec4{ 0.f }, dst, data.offset);
					else
						SetData(glm::vec4{ it->second }, dst, data.offset);
				}
				else
				{
					std::memset(dst.data() + data.offset, 0, data.size);
					auto it = vectorArrs.find(data.name);
					if (it != vectorArrs.end())
					{
						if (data.idx < it->second.size())
							SetData(it->second[data.idx], dst, data.offset);
					}
				}
			}
			else if (data.type == core::reflection::GetType<glm::mat4>())
			{
				auto it = mats.find(data.name);
				if (it == mats.end())
				{
					glm::mat4 value{ 0.f };
					SetData(value, dst, data.offset);
				}
				else
				{
					SetData(it->second, dst, data.offset);
				}
			}
		}
	}
	void Material::SetDefaultProperties()
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto blocks = { &pass->GetVertexUniforms(), &pass->GetVertexUniforms() };
			for (auto currentBlocks : blocks)
			{
				for (auto& uniformBlock : *currentBlocks)
				{
					for (auto& uniform : uniformBlock.data)
					{
						if (uniform.type == core::reflection::GetType<int>())
						{
							if (uniform.size == sizeof(int))
								ints.insert_or_assign(uniform.name, 0);
							else // 배열
							{
								// todo
							}
						}
						else if (uniform.type == core::reflection::GetType<float>())
						{
							if (uniform.size == sizeof(float))
								floats.insert_or_assign(uniform.name, 0.f);
							else
							{
								std::size_t n = uniform.size / sizeof(float);
								SetFloatArray(uniform.name, std::vector<float>(n, 0.f));
							}
						}
						else if (uniform.type == core::reflection::GetType<glm::vec2>())
						{
							if (uniform.size == sizeof(glm::vec2))
								vectors.insert_or_assign(uniform.name, glm::vec4{ 0.f });
							else
							{
								std::size_t n = uniform.size / sizeof(glm::vec2);
								SetVectorArray(uniform.name, std::vector<glm::vec4>(n, glm::vec4{ 0.f }));
							}
						}
						else if (uniform.type == core::reflection::GetType<glm::vec3>())
						{
							if (uniform.size == sizeof(glm::vec3))
								vectors.insert_or_assign(uniform.name, glm::vec4{ 0.f });
							else
							{
								std::size_t n = uniform.size / sizeof(glm::vec3);
								SetVectorArray(uniform.name, std::vector<glm::vec4>(n, glm::vec4{ 0.f }));
							}
						}
						else if (uniform.type == core::reflection::GetType<glm::vec4>())
						{
							if (uniform.size == sizeof(glm::vec4))
								vectors.insert_or_assign(uniform.name, glm::vec4{ 0.f });
							else
							{
								std::size_t n = uniform.size / sizeof(glm::vec4);
								SetVectorArray(uniform.name, std::vector<glm::vec4>(n, glm::vec4{ 0.f }));
							}
						}
						else if (uniform.type == core::reflection::GetType<glm::mat4>())
							mats.insert_or_assign(uniform.name, glm::mat4{ 0.f });
					}
				}
			}
		}
		bBufferDirty = true;
	}

	SH_RENDER_API void Material::UpdateUniformBuffers()
	{
		if (!core::IsValid(shader))
			return;

		if (!bBufferDirty)
			return;

		int passIdx = 0;
		for (auto& passData : perPassData[core::ThreadType::Game])
		{
			std::vector<uint8_t> temp{};
			for (auto& [binding, buffer] : passData.vertShaderData)
			{
				temp.clear();
				temp.resize(buffer->GetSize());
				for (auto& uniformBlock : shader->GetPass(passIdx)->GetVertexUniforms())
				{
					if (uniformBlock.type == ShaderPass::UniformType::Object)
						continue;
					if (uniformBlock.binding != binding)
						continue;

					FillData(uniformBlock, temp, binding);
				}
				SetUniformData(passIdx, binding, temp.data(), ShaderPass::ShaderStage::Vertex);
			}
			temp.clear();
			for (auto& [binding, buffer] : passData.fragShaderData)
			{
				temp.resize(buffer->GetSize());
				for (auto& uniformBlock : shader->GetPass(passIdx)->GetFragmentUniforms())
				{
					if (uniformBlock.type == ShaderPass::UniformType::Object)
						continue;
					if (uniformBlock.binding != binding)
						continue;

					FillData(uniformBlock, temp, binding);
				}
				SetUniformData(passIdx, binding, temp.data(), ShaderPass::ShaderStage::Fragment);
			}
			for (auto& uniformData : shader->GetPass(passIdx)->GetSamplerUniforms())
			{
				auto it = textures.find(uniformData.name);
				if (it == textures.end())
					continue;

				SetTextureData(passIdx, uniformData.binding, it->second);
			}
			++passIdx;
		}

		bBufferDirty = false;
	}

	SH_RENDER_API void Material::SetDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);

		bDirty = true;
	}
	SH_RENDER_API void Material::Sync()
	{
		if (bBufferSync)
		{
			std::swap(perPassData[core::ThreadType::Game], perPassData[core::ThreadType::Render]);
		}
		bBufferSync = false;
		bDirty = false;
	}

	SH_RENDER_API auto Material::GetShaderBuffer(std::size_t passIdx, ShaderPass::ShaderStage stage, uint32_t binding, core::ThreadType thr) const -> IBuffer*
	{
		auto& passUniformData = perPassData[thr][passIdx];
		if (stage == ShaderPass::ShaderStage::Vertex)
		{
			auto it = passUniformData.vertShaderData.find(binding);
			if (it == passUniformData.vertShaderData.end())
				return nullptr;
			else
				return it->second.get();
		}
		else
		{
			auto it = passUniformData.fragShaderData.find(binding);
			if (it == passUniformData.fragShaderData.end())
				return nullptr;
			else
				return it->second.get();
		}
		return nullptr;
	}
	SH_RENDER_API auto Material::GetUniformBuffer(std::size_t passIdx, core::ThreadType thr) const -> IUniformBuffer*
	{
		return perPassData[thr][passIdx].uniformBuffer.get();
	}

	SH_RENDER_API void Material::SetInt(const std::string& name, int value)
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				ints.insert_or_assign(name, value);
				bBufferDirty = true;
				break;
			}
		}
	}
	SH_RENDER_API auto Material::GetInt(const std::string& name) const -> int
	{
		auto it = ints.find(name);
		if (it == ints.end())
			return 0;
		return it->second;
	}
	SH_RENDER_API void Material::SetFloat(const std::string& name, float value)
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				floats.insert_or_assign(name, value);
				bBufferDirty = true;
				break;
			}
		}
	}
	SH_RENDER_API auto Material::GetFloat(const std::string& name) const -> float
	{
		auto it = floats.find(name);
		if (it == floats.end())
			return 0.f;
		return it->second;
	}
	SH_RENDER_API void Material::SetVector(const std::string& name, const glm::vec4& value)
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto uniform = pass->GetUniformBinding(name);
			if (uniform)
			{
				vectors.insert_or_assign(name, value);
				bBufferDirty = true;
				break;
			}
		}
	}
	SH_RENDER_API auto Material::GetVector(const std::string& name) const -> const glm::vec4*
	{
		auto it = vectors.find(name);
		if (it == vectors.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API void Material::SetMatrix(const std::string& name, const glm::mat4& value)
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				mats.insert_or_assign(name, value);
				bBufferDirty = true;
			}
		}
	}
	SH_RENDER_API auto Material::GetMatrix(const std::string& name) const -> const glm::mat4*
	{
		auto it = mats.find(name);
		if (it == mats.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API void Material::SetFloatArray(const std::string& name, const std::vector<float>& value)
	{
		if (!core::IsValid(shader))
			return;
		if (floatArr == nullptr)
			floatArr = std::make_unique<core::SMap<std::string, std::vector<float>, 4>>();

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				floatArr->insert_or_assign(name, value);
				bBufferDirty = true;
				break;
			}
		}
	}
	SH_RENDER_API void Material::SetFloatArray(const std::string& name, std::vector<float>&& value)
	{
		if (!core::IsValid(shader))
			return;
		if (floatArr == nullptr)
			floatArr = std::make_unique<core::SMap<std::string, std::vector<float>, 4>>();

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				floatArr->insert_or_assign(name, std::move(value));
				bBufferDirty = true;
			}
		}
	}
	SH_RENDER_API auto Material::GetFloatArray(const std::string& name) const -> const std::vector<float>*
	{
		if (floatArr == nullptr)
			return nullptr;
		auto it = floatArr->find(name);
		if (it == floatArr->end())
			return nullptr;
		return &it->second;
	}

	SH_RENDER_API void Material::SetVectorArray(const std::string& name, const std::vector<glm::vec4>& value)
	{
		if (!core::IsValid(shader))
			return;

		for (auto& pass : shader->GetPasses())
		{
			auto binding = pass->GetUniformBinding(name);
			if (binding)
			{
				vectorArrs.insert_or_assign(name, value);
				bBufferDirty = true;
			}
		}
	}
	SH_RENDER_API auto Material::GetVectorArray(const std::string& name) const -> const std::vector<glm::vec4>*
	{
		auto it = vectorArrs.find(name);
		if (it == vectorArrs.end())
			return nullptr;
		return &it->second;
	}

	SH_RENDER_API void Material::SetTexture(const std::string& name, Texture* tex)
	{
		if (!core::IsValid(shader))
			return;

		textures.insert_or_assign(name, tex);
		bBufferDirty = true;
	}
	SH_RENDER_API auto Material::GetTexture(const std::string& name) const -> Texture*
	{
		auto it = textures.find(name);
		if (it == textures.end())
			return nullptr;
		return it->second;
	}

	SH_RENDER_API bool Material::HasIntProperty(const std::string& name) const
	{
		auto it = ints.find(name);
		return it != ints.end();
	}
	SH_RENDER_API bool Material::HasFloatProperty(const std::string& name) const
	{
		auto it = floats.find(name);
		return it != floats.end();
	}
	SH_RENDER_API bool Material::HasVectorProperty(const std::string& name) const
	{
		auto it = vectors.find(name);
		return it != vectors.end();
	}
	SH_RENDER_API bool Material::HasMatrixProperty(const std::string& name) const
	{
		auto it = mats.find(name);
		return it != mats.end();
	}
	SH_RENDER_API bool Material::HasFloatArrayProperty(const std::string& name) const
	{
		if (floatArr == nullptr)
			return false;
		auto it = floatArr->find(name);
		return it != floatArr->end();
	}
	SH_RENDER_API bool Material::HasVectorArrayProperty(const std::string& name) const
	{
		auto it = vectorArrs.find(name);
		return it != vectorArrs.end();
	}
	SH_RENDER_API bool Material::HasTextureProperty(const std::string& name) const
	{
		auto it = textures.find(name);
		return it != textures.end();
	}

	SH_RENDER_API void Material::OnPropertyChanged(const core::reflection::Property& prop)
	{
		bBufferDirty = true;
	}

	SH_RENDER_API auto Material::Serialize() const -> core::Json
	{
		core::Json mainJson{ Super::Serialize() };
		if (core::IsValid(shader))
		{
			core::Json propertiesJson{};

			core::Json intJson{};
			for (auto& [name, value] : ints)
				intJson[name] = value;
			propertiesJson["ints"] = intJson;

			core::Json floatJson{};
			for (auto& [name, value] : floats)
				floatJson[name] = value;
			propertiesJson["floats"] = floatJson;

			core::Json vectorJson{};
			for (auto& [name, value] : vectors)
				vectorJson[name] = { value.x, value.y, value.z, value.w };
			propertiesJson["vectors"] = vectorJson;

			core::Json texJson{};
			for (auto& [name, value] : textures)
				texJson[name] = value->GetUUID().ToString();
			propertiesJson["textures"] = texJson;

			mainJson["properties"] = propertiesJson;
		}
		return mainJson;
	}
	SH_RENDER_API void Material::Deserialize(const core::Json& json)
	{
		Clean();

		Super::Deserialize(json);

		if (json.contains("name"))
		{
			std::string name = json["name"].get<std::string>();
			SetName(name);
		}
		if (json.contains("shader"))
		{
			std::string shaderUuid = json["shader"].get<std::string>();
			SObject* shaderObj = core::SObjectManager::GetInstance()->GetSObject(shaderUuid);
			if (core::IsValid(shaderObj))
			{
				if (shaderObj->GetType() == Shader::GetStaticType())
					SetShader(static_cast<Shader*>(shaderObj));
			}
		}
		if (json.contains("properties"))
		{
			const auto& propertiesJson = json["properties"];

			if (propertiesJson.contains("ints"))
			{
				const auto& intJson = propertiesJson["ints"];
				for (const auto& [name, value] : intJson.items())
				{
					int intValue = value.get<int>();
					SetInt(name, intValue);
				}
			}
			if (propertiesJson.contains("floats"))
			{
				const auto& floatJson = propertiesJson["floats"];
				for (const auto& [name, value] : floatJson.items())
				{
					float floatValue = value.get<float>();
					SetFloat(name, floatValue);
				}
			}
			if (propertiesJson.contains("vectors"))
			{
				const auto& vectorJson = propertiesJson["vectors"];
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
						SetVector(name, vecValue);
					}
				}
			}
			if (propertiesJson.contains("textures"))
			{
				const auto& texJson = propertiesJson["textures"];
				for (const auto& [name, value] : texJson.items())
				{
					std::string uuid = value.get<std::string>();
					auto ptr = core::SObjectManager::GetInstance()->GetSObject(uuid);
					if (!core::IsValid(ptr))
						continue;
					if (ptr->GetType() == Texture::GetStaticType())
						SetTexture(name, static_cast<Texture*>(ptr));
				}
			}
		}
	}
}//namespace