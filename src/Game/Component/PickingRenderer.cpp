#include "PCH.h"
#include "Component/PickingRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

#include "Render/Mesh.h"

namespace sh::game
{
	SH_GAME_API PickingRenderer::PickingRenderer(GameObject& owner) :
		MeshRenderer(owner)
	{
		id = PickingIdManager::AssignId(this);
		//SH_INFO_FORMAT("ID: {}", id);

		this->mat = world.materials.GetResource("PickingMaterial");
		renderer = owner.GetComponent<MeshRenderer>();
		if (core::IsValid(renderer))
		{
			auto mesh = renderer->GetMesh();
			if (core::IsValid(mesh))
				this->mesh = mesh;
		}

		SetMaterialPropertyBlock(SObject::Create<render::MaterialPropertyBlock>());
		SetRenderTagId(1 << 31);
	}
	SH_GAME_API PickingRenderer::~PickingRenderer()
	{
		PickingIdManager::Erase(id);
	}

	SH_GAME_API void PickingRenderer::Awake()
	{
		
	}
	SH_GAME_API void PickingRenderer::BeginUpdate()
	{
		if (!core::IsValid(camera))
			return;
		if (core::IsValid(renderer))
		{
			if (this->mesh != renderer->GetMesh())
				SetMesh(renderer->GetMesh());
		}

		if (!camera->pickingCallback.Empty())
		{
			float r = ((id & 0x000000FF) >> 0) / 255.0f;
			float g = ((id & 0x0000FF00) >> 8) / 255.0f;
			float b = ((id & 0x00FF0000) >> 16) / 255.0f;
			float a = 1.f;

			auto propertyBlock = GetMaterialPropertyBlock();
			propertyBlock->SetProperty("id", glm::vec4{ r, g, b, a });

			
		}
	}

	SH_GAME_API void PickingRenderer::SetCamera(PickingCamera& camera)
	{
		this->camera = &camera;
	}

	SH_GAME_API void PickingRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == "camera")
			SetCamera(*camera);
	}

	SH_GAME_API auto PickingIdManager::AssignId(PickingRenderer* renderer) -> uint32_t
	{
		if (emptyId.empty())
		{
			ids.insert({ nextId, renderer });
			return nextId++;
		}
		else
		{
			uint32_t id = emptyId.front();
			emptyId.pop();
			ids.insert({ id, renderer });
			return id;
		}
	}
	SH_GAME_API auto PickingIdManager::Get(uint32_t id) -> PickingRenderer*
	{
		auto it = ids.find(id);
		if (it == ids.end())
			return nullptr;
		return it->second;
	}
	SH_GAME_API void PickingIdManager::Erase(uint32_t id)
	{
		auto it = ids.find(id);
		if (it == ids.end())
			return;
		ids.erase(it);
		emptyId.push(id);
	}
}//namespace