#include "PCH.h"
#include "Component/MeshRenderer.h"
#include "Component/Camera.h"

#include "gameObject.h"

#include "Core/Reflection.hpp"

#include "Render/DrawableFactory.h"
#include "Render/IDrawable.h"
#include "Render/IUniformBuffer.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner),

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
			render::IDrawable* drawable = render::DrawableFactory::Create(gameObject.world.renderer);
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
		for (auto& [cam, drawable] : drawables)
		{
			drawable->Build(cam->GetNative(), *mesh, mat);
		}
	}

	void MeshRenderer::Awake()
	{
		Super::Awake();

		if (!core::IsValid(mat))
		{
			mat = gameObject.world.materials.GetResource("ErrorMaterial");
		}

		for(auto cam : gameObject.world.GetCameras())
			CreateDrawable(cam);

		gameObject.world.onCameraAdd.Register(onCameraAddListener);
		gameObject.world.onCameraRemove.Register(onCameraRemoveListener);
	}

	void MeshRenderer::Start()
	{
	}

	void MeshRenderer::FillData(const render::Shader::UniformData& uniform, std::vector<unsigned char>& uniformData, Camera* cam)
	{
		if (uniform.typeName == sh::core::reflection::GetTypeName<glm::mat4>())
		{
			if (uniform.name == "proj")
				std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetProjMatrix()[0], sizeof(glm::mat4));
			else if (uniform.name == "view")
				std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetViewMatrix()[0], sizeof(glm::mat4));
			else if (uniform.name == "model")
				std::memcpy(uniformCopyData.data() + uniform.offset, &gameObject.transform->localToWorldMatrix[0], sizeof(glm::mat4));
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

	void MeshRenderer::Update()
	{
		if (!sh::core::IsValid(mesh))
			return;
		if (!sh::core::IsValid(mat))
			return;
		if (!sh::core::IsValid(mat->GetShader()))
			return;

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->IsPause())
			return;

		render::Shader* shader = mat->GetShader();
		if (!core::IsValid(shader))
			return;

		mat->UpdateUniformBuffers();

		for (auto&[cam, drawable] : drawables)
		{
			if (!cam->active)
				continue;

			struct alignas(16) Uniform
			{
				glm::mat4 model;
				glm::mat4 view;
				glm::mat4 proj;
			} uniform{};
			uniform.model = gameObject.transform->localToWorldMatrix;
			uniform.view = cam->GetViewMatrix();
			uniform.proj = cam->GetProjMatrix();

			drawable->SetUniformData(0, &uniform, render::IDrawable::Stage::Vertex);

			gameObject.world.renderer.PushDrawAble(drawable);
		}//drawables
	}

#if SH_EDITOR
	SH_GAME_API void MeshRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (std::strcmp(prop.GetName(), "mesh") == 0)
		{
			for (auto cam : gameObject.world.GetCameras())
				CreateDrawable(cam);
		}
	}
#endif
}//namespace