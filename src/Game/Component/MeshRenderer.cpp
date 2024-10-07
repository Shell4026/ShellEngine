#include "PCH.h"
#include "Component/MeshRenderer.h"
#include "Component/Camera.h"

#include "gameObject.h"

#include "Core/Reflection.hpp"

#include "Render/DrawableFactory.h"
#include "Render/IDrawable.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	MeshRenderer::MeshRenderer() :
		mesh(nullptr), mat(nullptr),
		onCameraAddListener
		(
			[&](Camera* cam)
			{
				CreateDrawable(cam);
			}
		),
		onCameraRemoveListener
		(
			[&](Camera* cam)
			{
				auto it = drawables.find(cam);
				if (it == drawables.end())
					return;

				drawables.erase(it);
			}
		)
	{
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetMesh(sh::render::Mesh& mesh)
	{
		this->mesh = &mesh;
	}

	auto MeshRenderer::GetMesh() const -> const sh::render::Mesh&
	{
		return *mesh;
	}

	void MeshRenderer::SetMaterial(sh::render::Material& mat)
	{
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
			render::IDrawable* drawable = render::DrawableFactory::Create(gameObject->world.renderer);
			drawable->Build(camera->GetNative(), *mesh, mat);
			drawables.insert({ camera, drawable });
		}
		else
		{
			it->second->Build(camera->GetNative(), *mesh, mat);
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

		gameObject->world.onCameraAdd.Register(onCameraAddListener);
		gameObject->world.onCameraRemove.Register(onCameraRemoveListener);
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
		if (renderer->IsPause())
			return;

		for (auto&[cam, drawable] : drawables)
		{
			if (!cam->active)
				continue;
			
			std::array<const render::Shader::UniformMap*, 2> uniforms{ &mat->GetShader()->vertexUniforms ,  &mat->GetShader()->fragmentUniforms };
			// 셰이더 유니폼 값 전달
			for (int stageIdx = 0; stageIdx < 2; ++stageIdx) // 0 버텍스 스테이지 1 프레그먼트 스테이지
			{
				auto stage = render::IDrawable::Stage::Vertex;
				if(stageIdx == 1)
					stage = render::IDrawable::Stage::Fragment;
				for (auto& [id, uniformVec] : *uniforms[stageIdx])
				{
					size_t size = uniformVec.back().offset + uniformVec.back().size;
					uniformCopyData.resize(size);
					for (const auto& uniform : uniformVec)
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

					drawable->SetUniformData(id, uniformCopyData.data(), stage);
				}//for uniformPair
			}//for i

			// 텍스쳐
			for (auto& sampler : mat->GetShader()->samplerFragmentUniforms)
			{
				auto tex = mat->GetTexture(sampler.second.name);
				if (tex != nullptr)
					drawable->SetTextureData(sampler.first, tex);
			}
			gameObject->world.renderer.PushDrawAble(drawable);
		}//drawables
	}
}//namespace