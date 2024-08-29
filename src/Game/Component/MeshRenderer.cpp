#include "Component/MeshRenderer.h"

#include "Component/Camera.h"

#include "gameObject.h"

#include "Core/Reflection.hpp"

#include "Render/VulkanRenderer.h"
#include "Render/VulkanDrawable.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	MeshRenderer::MeshRenderer() :
		mesh(nullptr), mat(nullptr)
	{
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetMesh(sh::render::Mesh& mesh)
	{
		if(core::IsValid(this->mesh))
			gameObject->world.meshes.DestroyNotifies(this, this->mesh);
		this->mesh = &mesh;
	}

	auto MeshRenderer::GetMesh() const -> const sh::render::Mesh&
	{
		return *mesh;
	}

	void MeshRenderer::SetMaterial(sh::render::Material& mat)
	{
		if (core::IsValid(this->mat))
			gameObject->world.materials.DestroyNotifies(this, this->mat);
		this->mat = &mat;
	}

	auto MeshRenderer::GetMaterial() const -> const sh::render::Material&
	{
		return *mat;
	}

	void MeshRenderer::CreateDrawable(Camera* camera)
	{
		if (!core::IsValid(mesh))
		{
			mesh = nullptr;
			return;
		}
		if (!core::IsValid(mat))
		{
			mat = nullptr;
			return;
		}
		if (!core::IsValid(mat->GetShader()))
			return;
		if (!core::IsValid(camera))
			return;

		auto it = drawables.find(camera);
		if (it == drawables.end())
		{

			render::VulkanRenderer& renderer = static_cast<render::VulkanRenderer&>(gameObject->world.renderer);
			std::unique_ptr<render::IDrawable> drawable = std::make_unique<sh::render::VulkanDrawable>(renderer);
			gc->SetRootSet(drawable.get());

			if (camera->renderTexture != nullptr)
				drawable->SetFramebuffer(*camera->renderTexture->GetFramebuffer());

			drawable->Build(mesh, mat);
			drawables.insert({ camera, std::move(drawable) });
		}
		else
		{
			it->second->Build(mesh, mat);
		}
	}
	void MeshRenderer::RebuildDrawables()
	{
		for (auto cam : gameObject->world.GetCameras())
		{
			CreateDrawable(cam);
		}
	}

	void MeshRenderer::Awake()
	{
		Super::Awake();
		for(auto cam : gameObject->world.GetCameras())
			CreateDrawable(cam);
	}

	void MeshRenderer::Start()
	{
		for (auto cam : gameObject->world.GetCameras())
		{
			if (drawables.find(cam) == drawables.end())
			{
				CreateDrawable(cam);
			}
		}
	}

	void MeshRenderer::Update()
	{
		if (!sh::core::IsValid(mesh))
			return;
		if (!sh::core::IsValid(mat))
			return;
		if (!sh::core::IsValid(mat->GetShader()))
			return;

		sh::render::Renderer* renderer = &gameObject->world.renderer;
		if (renderer->IsPause())
			return;

		for (auto cam : gameObject->world.GetCameras())
		{
			if (drawables.find(cam) == drawables.end())
			{
				CreateDrawable(cam);
			}
		}
		//++it 필수
		for (auto it = drawables.begin(); it != drawables.end();)
		{
			Camera* cam = it->first;
			auto& drawable = it->second;
			if (!sh::core::IsValid(cam))
			{
				it = drawables.erase(it);
				continue;
			}
			if (!cam->active)
			{
				++it;
				continue;
			}
			if (cam->renderTexture != nullptr)
			{
				drawable->SetFramebuffer(*cam->renderTexture->GetFramebuffer());
			}
			
			std::array<const render::Shader::UniformMap*, 2> uniforms{ &mat->GetShader()->vertexUniforms ,  &mat->GetShader()->fragmentUniforms };
			//셰이더 유니폼 값 전달
			for (int i = 0; i < 2; ++i)
			{
				auto stage = render::IDrawable::Stage::Vertex;
				if(i == 1)
					stage = render::IDrawable::Stage::Fragment;
				for (auto& uniformPair : *uniforms[i])
				{
					size_t size = uniformPair.second.back().offset + uniformPair.second.back().size;
					uniformCopyData.resize(size);
					for (const auto& uniform : uniformPair.second)
					{
						if (uniform.typeName == sh::core::reflection::GetTypeName<glm::mat4>())
						{
							if (uniform.name == "proj")
								std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetProjMatrix()[0], sizeof(glm::mat4));
							else if (uniform.name == "view")
								std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetViewMatrix()[0], sizeof(glm::mat4));
							else if (uniform.name == "model")
								std::memcpy(uniformCopyData.data() + uniform.offset, &gameObject->transform->localToWorldMatrix[0], sizeof(glm::mat4));
							else
							{
								auto matrix = mat->GetMatrix(uniform.name);
								if (matrix == nullptr)
								{
									glm::mat4 defaultMat{ 0.f };
									std::memcpy(uniformCopyData.data() + uniform.offset, &defaultMat[0], sizeof(glm::mat4));
								}
								else
									std::memcpy(uniformCopyData.data() + uniform.offset, &matrix[0], sizeof(glm::mat4));
							}
						}
						else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec4>())
						{
							auto vec = mat->GetVector(uniform.name);
							if (vec == nullptr)
								SetUniformData(glm::vec4{ 0.f }, uniformCopyData, uniform.offset);
							else
								SetUniformData(*vec, uniformCopyData, uniform.offset);
						}
						else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec3>())
						{
							auto vec = mat->GetVector(uniform.name);
							if (vec == nullptr)
								SetUniformData(glm::vec3{ 0.f }, uniformCopyData, uniform.offset);
							else
								SetUniformData(glm::vec3{ *vec }, uniformCopyData, uniform.offset);
						}
						else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec2>())
						{
							auto vec = mat->GetVector(uniform.name);
							if (vec == nullptr)
								SetUniformData(glm::vec2{ 0.f }, uniformCopyData, uniform.offset);
							else
								SetUniformData(glm::vec2{ *vec }, uniformCopyData, uniform.offset);
						}
						else if (uniform.typeName == sh::core::reflection::GetTypeName<float>())
						{
							float value = mat->GetFloat(uniform.name);
							SetUniformData(value, uniformCopyData, uniform.offset);
						}
					}

					drawable->SetUniformData(uniformPair.first, uniformCopyData.data(), stage);
				}//for uniformPair
			}//for i
			//텍스쳐
			for (auto& sampler : mat->GetShader()->samplerFragmentUniforms)
			{
				auto tex = mat->GetTexture(sampler.second.name);
				if (tex != nullptr)
					drawable->SetTextureData(sampler.first, tex);
			}
			//std::cout << "push: " << this << ' ' << "cam: " << cam->GetCameraHandle() << '\n';
			gameObject->world.renderer.PushDrawAble(drawable.get(), cam->GetCameraHandle());
			++it;
		}
	}
}//namespace