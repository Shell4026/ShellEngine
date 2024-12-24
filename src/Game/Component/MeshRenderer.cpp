#include "PCH.h"
#include "Component/MeshRenderer.h"
#include "Component/Camera.h"
#include "Component/PickingRenderer.h"
#include "Component/PointLight.h"

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
		onMatrixUpdateListener.SetCallback([&](const glm::mat4& mat)
			{
				if (core::IsValid(mesh))
					worldAABB = mesh->GetBoundingBox().GetWorldAABB(mat);
			}
		);
		gameObject.transform->onMatrixUpdate.Register(onMatrixUpdateListener);
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetMesh(sh::render::Mesh* mesh)
	{
		this->mesh = mesh;
		if (core::IsValid(mesh))
		{
			worldAABB = mesh->GetBoundingBox().GetWorldAABB(gameObject.transform->localToWorldMatrix);

			for (auto cam : gameObject.world.GetCameras())
				CreateDrawable(cam);

#if SH_EDITOR
			if (GetType() == MeshRenderer::GetStaticType())
			{
				if (auto picking = gameObject.GetComponent<PickingRenderer>(); picking == nullptr)
				{
					picking = gameObject.AddComponent<PickingRenderer>();
					picking->hideInspector = true;
					picking->SetMesh(mesh);
					picking->SetCamera(*world.GetGameObject("PickingCamera")->GetComponent<PickingCamera>());
				}
				else
					picking->SetMesh(mesh);
			}
#endif
		}
	}

	SH_GAME_API auto MeshRenderer::GetMesh() const -> const sh::render::Mesh*
	{
		return mesh;
	}
	SH_GAME_API auto MeshRenderer::GetMesh() -> sh::render::Mesh*
	{
		return mesh;
	}

	void MeshRenderer::SetMaterial(sh::render::Material* mat)
	{
		this->mat = mat;
		if (core::IsValid(mat))
		{
			auto shader = this->mat->GetShader();
			if (shader->GetUniformBinding("lightCount"))
				bShaderHasLight = true;
			else
				bShaderHasLight = false;

			for (auto cam : gameObject.world.GetCameras())
				CreateDrawable(cam);
		}
	}

	auto MeshRenderer::GetMaterial() const -> sh::render::Material*
	{
		return mat;
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
			drawable->Build(camera->GetNative(), mesh, mat);
			drawables.insert({ camera, drawable });
		}
		else
		{
			it->second->Build(camera->GetNative(), mesh, mat);
		}
	}
	void MeshRenderer::RebuildDrawables()
	{
		for (auto& [cam, drawable] : drawables)
		{
			drawable->Build(cam->GetNative(), mesh, mat);
		}
	}
	void MeshRenderer::CleanDrawables()
	{
		drawables.clear();
	}

	void MeshRenderer::Awake()
	{
		Super::Awake();

		if (!core::IsValid(mat))
			mat = gameObject.world.materials.GetResource("ErrorMaterial");

		for(auto cam : gameObject.world.GetCameras())
			CreateDrawable(cam);

		gameObject.world.onCameraAdd.Register(onCameraAddListener);
		gameObject.world.onCameraRemove.Register(onCameraRemoveListener);
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

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->IsPause())
			return;

		render::Shader* shader = mat->GetShader();
		if (!core::IsValid(shader))
			return;
	
		mat->UpdateUniformBuffers();

		// 광원 구조체 채우기
		struct
		{
			alignas(16) glm::vec4 lightPosRange[10];
			alignas(16) int lightCount = 0;
		} lightStruct;
		if (bShaderHasLight)
		{
			std::fill(lightStruct.lightPosRange, lightStruct.lightPosRange + 10, glm::vec4{ 0.f });
			auto lights = world.GetLightOctree().Query(worldAABB);
			if (lights.size() < 10)
			{
				int idx = 0;
				for (int i = 0; i < lights.size(); ++i)
				{
					ILight* light = static_cast<ILight*>(lights[i]);
					const Vec3& pos = light->gameObject.transform->GetWorldPosition();
					if (light->GetType() == PointLight::GetStaticType())
						lightStruct.lightPosRange[idx++] = { pos.x, pos.y, pos.z, static_cast<PointLight*>(light)->GetRadius() };
				}
				lightStruct.lightCount = idx;
			}
		}

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
			if (bShaderHasLight)
				drawable->SetUniformData(1, &lightStruct, render::IDrawable::Stage::Fragment);

			gameObject.world.renderer.PushDrawAble(drawable);
		}//drawables
	}

	SH_GAME_API void MeshRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "mesh")
		{
			SetMesh(mesh);
		}
		else if (prop.GetName() == "mat")
		{
			SetMaterial(mat);
		}
	}
}//namespace