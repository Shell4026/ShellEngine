#include "pch.h"
#include "Material.h"

#include "Renderer.h"
#include "BufferFactory.h"
#include "IBuffer.h"
#include "IUniformBuffer.h"

#include "Core/Util.h"

namespace sh::render
{
	SH_RENDER_API Material::Material() :
		renderer(nullptr),
		shader(nullptr),
		bDirty(false), bBufferDirty(false), bBufferSync(false)
	{
	}
	SH_RENDER_API Material::Material(Shader* shader) :
		renderer(nullptr),
		shader(shader),
		bDirty(false), bBufferDirty(false), bBufferSync(false)
	{
	}

	SH_RENDER_API Material::~Material()
	{
		Clean();
	}

	SH_RENDER_API Material::Material(Material&& other) noexcept :
		renderer(other.renderer),
		shader(other.shader),
		bDirty(other.bDirty), bBufferDirty(other.bBufferDirty), bBufferSync(other.bBufferSync)
	{
		other.renderer = nullptr;
		other.shader = nullptr;

		other.bDirty = false;
		other.bBufferDirty = false;
		other.bBufferSync = false;
	}

	void Material::Clean()
	{
		for (int thr = 0; thr < vertBuffers.size(); ++thr)
		{
			vertBuffers[thr].clear();
			fragBuffers[thr].clear();
			uniformBuffer[thr].reset();
		}
		floats.clear();
		ints.clear();
		vectors.clear();
		mats.clear();
	}

	SH_RENDER_API void Material::SetShader(Shader* shader)
	{
		this->shader = shader;
		if (!core::IsValid(this->shader))
			return;
		if (renderer == nullptr)
			return;

		Clean();
		Build(*renderer);
	}
	
	SH_RENDER_API auto Material::GetShader() const -> Shader*
	{
		return shader;
	}

	SH_RENDER_API void Material::Build(const Renderer& renderer)
	{
		if (!core::IsValid(shader))
			return;

		this->renderer = &renderer;

		// 버텍스 유니폼
		for (auto& uniformBlock : shader->GetVertexUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Object)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			for (std::size_t thr = 0; thr < vertBuffers.size(); ++thr)
				vertBuffers[thr].insert({ uniformBlock.binding, BufferFactory::Create(renderer, size) });
		}
		// 픽셀 유니폼
		for (auto& uniformBlock : shader->GetFragmentUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Object)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			for (std::size_t thr = 0; thr < fragBuffers.size(); ++thr)
				fragBuffers[thr].insert({ uniformBlock.binding, BufferFactory::Create(renderer, size) });
		}

		for (std::size_t thr = 0; thr < uniformBuffer.size(); ++thr)
			uniformBuffer[thr] = BufferFactory::CreateUniformBuffer(renderer, *shader, Shader::UniformType::Material);
	}

	void Material::SetUniformData(uint32_t binding, const void* data, Stage stage)
	{
		if (stage == Stage::Vertex)
		{
			auto it = vertBuffers[core::ThreadType::Game].find(binding);
			if (it == vertBuffers[core::ThreadType::Game].end())
				return;

			it->second->SetData(data);
			uniformBuffer[core::ThreadType::Game]->Update(binding, *it->second);
			
			bBufferSync = true;
		}
		else if (stage == Stage::Fragment)
		{
			auto it = fragBuffers[core::ThreadType::Game].find(binding);
			if (it == fragBuffers[core::ThreadType::Game].end())
				return;

			it->second->SetData(data);
			uniformBuffer[core::ThreadType::Game]->Update(binding, *it->second);

			bBufferSync = true;
		}
		SetDirty();
	}
	void Material::SetTextureData(uint32_t binding, Texture* tex)
	{
		if (!core::IsValid(tex) || renderer == nullptr)
			return;

		bool find = false;
		for (auto& uniform : shader->GetSamplerUniforms())
		{
			if (uniform.binding == binding)
			{
				find = true;
				break;
			}
		}
		if (!find)
			return;

		uniformBuffer[core::ThreadType::Game]->Update(binding, *tex);

		bBufferSync = true;
		SetDirty();
	}

	void Material::FillData(const Shader::UniformBlock& uniformBlock, std::vector<uint8_t>& dst, uint32_t binding)
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

	SH_RENDER_API void Material::UpdateUniformBuffers()
	{
		if (!core::IsValid(shader))
			return;

		if (!bBufferDirty)
			return;

		std::vector<uint8_t> temp;

		for (auto& [binding, buffer] : vertBuffers[core::ThreadType::Game])
		{
			temp.clear();
			temp.resize(buffer->GetSize());
			for (auto& uniformBlock : shader->GetVertexUniforms())
			{
				if (uniformBlock.type == Shader::UniformType::Object)
					continue;
				if (uniformBlock.binding != binding)
					continue;

				FillData(uniformBlock, temp, binding);
			}
			SetUniformData(binding, temp.data(), Stage::Vertex);
		}
		temp.clear();
		for (auto& [binding, buffer] : fragBuffers[core::ThreadType::Game])
		{
			temp.resize(buffer->GetSize());
			for (auto& uniformBlock : shader->GetFragmentUniforms())
			{
				if (uniformBlock.type == Shader::UniformType::Object)
					continue;
				if (uniformBlock.binding != binding)
					continue;

				FillData(uniformBlock, temp, binding);
			}
			SetUniformData(binding, temp.data(), Stage::Fragment);
		}
		for (auto& uniform : shader->GetSamplerUniforms())
		{
			auto it = textures.find(uniform.binding);
			if (it == textures.end())
				continue;

			SetTextureData(uniform.binding, it->second);
		}

		bBufferDirty = false;
	}

	SH_RENDER_API void Material::SetDirty()
	{
		if (bDirty)
			return;
		renderer->GetThreadSyncManager().PushSyncable(*this);
		bDirty = true;
	}
	SH_RENDER_API void Material::Sync()
	{
		if (bBufferSync)
		{
			std::swap(vertBuffers[core::ThreadType::Game], vertBuffers[core::ThreadType::Render]);
			std::swap(fragBuffers[core::ThreadType::Game], fragBuffers[core::ThreadType::Render]);
			std::swap(uniformBuffer[core::ThreadType::Game], uniformBuffer[core::ThreadType::Render]);
		}
		bBufferSync = false;
		bDirty = false;
	}

	SH_RENDER_API auto Material::GetShaderBuffer(Stage stage, uint32_t binding, core::ThreadType thr) const -> IBuffer*
	{
		if (stage == Stage::Vertex)
		{
			auto it = vertBuffers[thr].find(binding);
			if (it == vertBuffers[thr].end())
				return nullptr;
			else
				return it->second.get();
		}
		else
		{
			auto it = fragBuffers[thr].find(binding);
			if (it == fragBuffers[thr].end())
				return nullptr;
			else
				return it->second.get();
		}
		return nullptr;
	}
	SH_RENDER_API auto Material::GetUniformBuffer(core::ThreadType thr) const -> IUniformBuffer*
	{
		return uniformBuffer[thr].get();
	}

	SH_RENDER_API void Material::SetInt(std::string_view name, int value)
	{
		if (!core::IsValid(shader))
			return;
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			ints.insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetInt(std::string_view name) const -> int
	{
		auto it = ints.find(std::string{ name });
		if (it == ints.end())
			return 0;
		return it->second;
	}
	SH_RENDER_API void Material::SetFloat(std::string_view name, float value)
	{
		if (!core::IsValid(shader))
			return;
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			floats.insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetFloat(std::string_view name) const -> float
	{
		auto it = floats.find(std::string{ name });
		if (it == floats.end())
			return 0.f;
		return it->second;
	}
	SH_RENDER_API void Material::SetVector(std::string_view name, const glm::vec4& value)
	{
		if (!core::IsValid(shader))
			return;
		auto uniform = shader->GetUniformBinding(name);
		if (uniform)
		{
			vectors.insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetVector(std::string_view name) const -> const glm::vec4*
	{
		auto it = vectors.find(std::string{ name });
		if (it == vectors.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API void Material::SetMatrix(std::string_view name, const glm::mat4& value)
	{
		if (!core::IsValid(shader))
			return;
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			mats.insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetMatrix(std::string_view name) const -> const glm::mat4*
	{
		auto it = mats.find(std::string{ name });
		if (it == mats.end())
			return nullptr;
		return &it->second;
	}
	SH_RENDER_API void Material::SetFloatArray(std::string_view name, const std::vector<float>& value)
	{
		if (!core::IsValid(shader))
			return;
		if (floatArr == nullptr)
			floatArr = std::make_unique<core::SMap<std::string, std::vector<float>, 4>>();
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			floatArr->insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetFloatArray(std::string_view name) const -> const std::vector<float>*
	{
		if (floatArr == nullptr)
			return nullptr;
		auto it = floatArr->find(std::string{ name });
		if (it == floatArr->end())
			return nullptr;
		return &it->second;
	}

	SH_RENDER_API void Material::SetVectorArray(std::string_view name, const std::vector<glm::vec4>& value)
	{
		if (!core::IsValid(shader))
			return;
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			vectorArrs.insert_or_assign(std::string{ name }, value);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetVectorArray(std::string_view name) const -> const std::vector<glm::vec4>*
	{
		auto it = vectorArrs.find(std::string{ name });
		if (it == vectorArrs.end())
			return nullptr;
		return &it->second;
	}

	SH_RENDER_API void Material::SetTexture(std::string_view name, Texture* tex)
	{
		if (!core::IsValid(shader))
			return;

		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			textures.insert_or_assign(binding.value(), tex);
			bBufferDirty = true;
		}
	}
	SH_RENDER_API auto Material::GetTexture(std::string_view name) const -> Texture*
	{
		auto binding = shader->GetUniformBinding(name);
		if (binding)
		{
			auto it = textures.find(binding.value());
			if (it == textures.end())
				return nullptr;
			return it->second;
		}
		return nullptr;
	}
}