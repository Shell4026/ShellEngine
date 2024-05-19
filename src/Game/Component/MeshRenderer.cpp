#include "Component/MeshRenderer.h"

#include "GameObject.h"

#include "Core/Util.h"

#include "Render/VulkanRenderer.h"
#include "Render/VulkanDrawable.h"

#include <iostream>

namespace sh::game
{
	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner),
		mesh(nullptr), mat(nullptr), drawable(nullptr)
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

	void MeshRenderer::CreateDrawable()
	{
		if (mesh == nullptr)
			return;

		auto it = drawables.find(mesh);
		if (it == drawables.end())
		{
			gameObject.world.meshes.RegisterDestroyNotify(mesh, [&]() {
				drawables.erase(mesh);
			});

			if (gameObject.world.renderer.apiType == sh::render::RenderAPI::Vulkan)
			{
				auto result = drawables.insert({mesh,
					std::make_unique<sh::render::VulkanDrawable>
					(static_cast<const sh::render::VulkanRenderer&>(gameObject.world.renderer)) 
				});

				auto drawable = static_cast<sh::render::VulkanDrawable*>(result.first->second.get());
				drawable->Build(mat, mesh);
				this->drawable = drawable;
			}
		}
		else
			this->drawable = it->second.get();
	}

	void MeshRenderer::Awake()
	{
		CreateDrawable();
	}

	void MeshRenderer::Start()
	{

	}

	void MeshRenderer::Update()
	{
		if (sh::core::IsValid(mesh))
		{
			if (drawable == nullptr)
				return;

			sh::render::Renderer* renderer = &gameObject.world.renderer;
			if (renderer->apiType == sh::render::RenderAPI::Vulkan)
			{
				int frameIdx = static_cast<sh::render::VulkanRenderer*>(renderer)->GetCurrentFrame();
				static_cast<sh::render::VulkanDrawable*>(drawable)->SetUniformData(frameIdx, mat->GetVector("offset"));
			}
			gameObject.world.renderer.PushDrawAble(drawable);
		}
	}
}