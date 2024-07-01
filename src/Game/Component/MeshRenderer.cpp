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

		if (gameObject->world.renderer.apiType == sh::render::RenderAPI::Vulkan)
		{
			render::VulkanRenderer& renderer = static_cast<render::VulkanRenderer&>(gameObject->world.renderer);
			auto drawable = std::make_unique<sh::render::VulkanDrawable>(renderer);
			if (camera->renderTexture != nullptr)
				drawable->SetFramebuffer(*camera->renderTexture->GetFramebuffer());

			drawable->Build(mesh, mat);
			drawables.insert({ camera, std::move(drawable) });
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
		if (renderer->isPause)
			return;

		for (auto cam : gameObject->world.GetCameras())
		{
			if (drawables.find(cam) == drawables.end())
			{
				CreateDrawable(cam);
			}
		}
		for (auto& pair : drawables)
		{
			Camera* cam = pair.first;
			auto& drawable = pair.second;
			if (!sh::core::IsValid(cam))
			{
				drawables.erase(cam);
				continue;
			}
			if (!cam->active)
				continue;

			if (cam->renderTexture != nullptr)
			{
				drawable->SetFramebuffer(*cam->renderTexture->GetFramebuffer());
			}

			//셰이더 유니폼 값 전달
			for (auto& uniforms : mat->GetShader()->vertexUniforms)
			{
				size_t size = uniforms.second.back().offset + uniforms.second.back().size;
				uniformCopyData.resize(size);
				for (const auto& uniform : uniforms.second)
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
						{
							glm::vec4 defaultVec{ 0.f };
							std::memcpy(uniformCopyData.data() + uniform.offset, &defaultVec, sizeof(glm::vec4));
						}
						else
							std::memcpy(uniformCopyData.data() + uniform.offset, vec, sizeof(glm::vec4));
					}
					else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec3>())
					{
						auto vec = mat->GetVector(uniform.name);
						if (vec == nullptr)
						{
							glm::vec3 defaultVec{ 0.f };
							std::memcpy(uniformCopyData.data() + uniform.offset, &defaultVec, sizeof(glm::vec3));
						}
						else
							std::memcpy(uniformCopyData.data() + uniform.offset, vec, sizeof(glm::vec3));
					}

					else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec2>())
					{
						auto vec = mat->GetVector(uniform.name);
						if (vec == nullptr)
						{
							glm::vec2 defaultVec{ 0.f };
							std::memcpy(uniformCopyData.data() + uniform.offset, &defaultVec, sizeof(glm::vec2));
						}
						else
							std::memcpy(uniformCopyData.data() + uniform.offset, vec, sizeof(glm::vec2));
					}
					else if (uniform.typeName == sh::core::reflection::GetTypeName<float>())
					{
						float value = mat->GetFloat(uniform.name);
						std::memcpy(uniformCopyData.data() + uniform.offset, &value, sizeof(float));
					}
				}

				drawable->SetUniformData(uniforms.first, uniformCopyData.data());
			}
			//텍스쳐
			for (auto& sampler : mat->GetShader()->samplerFragmentUniforms)
			{
				auto tex = mat->GetTexture(sampler.second.name);
				if (tex != nullptr)
					drawable->SetTextureData(sampler.first, tex);
			}
			//std::cout << "push: " << this << ' ' << "cam: " << cam->GetCameraHandle() << '\n';
			gameObject->world.renderer.PushDrawAble(drawable.get(), cam->GetCameraHandle());
		}
	}
}