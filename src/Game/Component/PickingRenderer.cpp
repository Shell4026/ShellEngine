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
		SH_INFO_FORMAT("ID: {}", id);

		this->mat = world.materials.GetResource("PickingMaterial");
		auto renderer = owner.GetComponent<MeshRenderer>();
		if (renderer)
		{
			auto mesh = renderer->GetMesh();
			if (core::IsValid(mesh))
			{
				this->mesh = mesh;
			}
		}
	}
	SH_GAME_API PickingRenderer::~PickingRenderer()
	{
		PickingIdManager::Erase(id);
	}

	SH_GAME_API void PickingRenderer::Awake()
	{
		
	}
	SH_GAME_API void PickingRenderer::Update()
	{
		for (auto& [cam, drawable] : drawables)
		{
			if (!cam->active)
				continue;

			float r = ((id & 0x000000FF) >> 0) / 255.0f;
			float g = ((id & 0x0000FF00) >> 8) / 255.0f;
			float b = ((id & 0x00FF0000) >> 16) / 255.0f;
			float a = 1.f;
			struct alignas(16) Uniform
			{
				glm::vec4 color;
			} uniform{ {r, g, b, a} };

			drawable->SetUniformData(1, &uniform, render::IDrawable::Stage::Fragment);
		}
		Super::Update();
	}

	SH_GAME_API void PickingRenderer::SetCamera(PickingCamera& camera)
	{
		this->camera = &camera;
		CleanDrawables();
		CreateDrawable(this->camera);
	}

	SH_GAME_API void PickingRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (std::strcmp(prop.GetName(), "mesh") == 0)
		{
			this->mesh = mesh;
		}
		if (std::strcmp(prop.GetName(), "camera") == 0)
		{
			SetCamera(*camera);
		}
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