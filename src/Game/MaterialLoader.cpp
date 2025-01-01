#include "PCH.h"
#include "MaterialLoader.h"

#include "Core/FileLoader.h"
#include "Core/ISerializable.h"

#include "Render/Shader.h"
#include "Render/Material.h"

namespace sh::game
{
	SH_GAME_API MaterialLoader::MaterialLoader(const render::IRenderContext& context) :
		context(context)
	{
	}

	void MaterialLoader::SetDefaultProperty(render::Material* mat, const render::Shader::UniformData& uniformData)
	{
		if (uniformData.type == core::reflection::GetType<int>())
		{
			if (!mat->HasIntProperty(uniformData.name))
				mat->SetInt(uniformData.name, 0);
		}
		else if (uniformData.type == core::reflection::GetType<float>())
		{
			if (uniformData.size == sizeof(float))
			{
				if (!mat->HasFloatProperty(uniformData.name))
					mat->SetFloat(uniformData.name, 0);
			}
			else
			{
				if (!mat->HasFloatArrayProperty(uniformData.name))
				{
					std::size_t count = uniformData.size / sizeof(float);
					std::vector<float> floats(count, 0.f);
					mat->SetFloatArray(uniformData.name, floats);
				}
			}
		}
		else if (uniformData.type == core::reflection::GetType<glm::vec2>())
		{
			if (uniformData.size == sizeof(glm::vec2))
			{
				if (!mat->HasVectorProperty(uniformData.name))
					mat->SetVector(uniformData.name, { 0.f, 0.f, 0.f, 0.f });
			}
			else
			{
				if (!mat->HasVectorArrayProperty(uniformData.name))
				{
					std::size_t count = uniformData.size / sizeof(glm::vec2);
					std::vector<glm::vec4> vecs{ count, glm::vec4{ 0.f } };
					mat->SetVectorArray(uniformData.name, vecs);
				}
			}
		}
		else if (uniformData.type == core::reflection::GetType<glm::vec3>())
		{
			if (uniformData.size == sizeof(glm::vec3))
			{
				if (!mat->HasVectorProperty(uniformData.name))
					mat->SetVector(uniformData.name, { 0.f, 0.f, 0.f, 0.f });
			}
			else
			{
				if (!mat->HasVectorArrayProperty(uniformData.name))
				{
					std::size_t count = uniformData.size / sizeof(glm::vec3);
					std::vector<glm::vec4> vecs{ count, glm::vec4{ 0.f } };
					mat->SetVectorArray(uniformData.name, vecs);
				}
			}
		}
		else if (uniformData.type == core::reflection::GetType<glm::vec4>())
		{
			if (uniformData.size == sizeof(glm::vec4))
			{
				if (!mat->HasVectorProperty(uniformData.name))
					mat->SetVector(uniformData.name, { 0.f, 0.f, 0.f, 0.f });
			}
			else
			{
				if (!mat->HasVectorArrayProperty(uniformData.name))
				{
					std::size_t count = uniformData.size / sizeof(glm::vec4);
					std::vector<glm::vec4> vecs{ count, glm::vec4{ 0.f } };
					mat->SetVectorArray(uniformData.name, vecs);
				}
			}
		}
		else if (uniformData.type == core::reflection::GetType<glm::mat4>())
		{
			if (!mat->HasMatrixProperty(uniformData.name))
				mat->SetMatrix(uniformData.name, glm::mat4{ 0.f });
		}
	}
	SH_GAME_API auto MaterialLoader::Load(std::string_view filename) -> render::Material*
	{
		auto file = core::FileLoader::LoadText(filename);
		if (!file.has_value())
			return nullptr;
		
		core::Json matJson{ core::Json::parse(file.value()) };

		if (!matJson.contains("type"))
			return nullptr;
		if (matJson["type"].get<std::string>() != "Material")
			return nullptr;

		render::Material* mat = core::SObject::Create<render::Material>();
		mat->Deserialize(matJson);
		render::Shader* shader = mat->GetShader();
		if (core::IsValid(shader))
		{
			for (auto& uniformBlock : shader->GetVertexUniforms())
			{
				for (auto& uniformData : uniformBlock.data)
					SetDefaultProperty(mat, uniformData);
			}
			for (auto& uniformBlock : shader->GetFragmentUniforms())
			{
				for (auto& uniformData : uniformBlock.data)
					SetDefaultProperty(mat, uniformData);
			}
			for (auto& uniformData : shader->GetSamplerUniforms())
			{
				if (!mat->HasTextureProperty(uniformData.name))
					mat->SetTexture(uniformData.name, nullptr);
			}
		}
		mat->Build(context);

		return mat;
	}
}//namespace