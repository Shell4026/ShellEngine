#include "Component/MeshRenderer.h"

#include "Component/Camera.h"

#include "GameObject.h"

#include "Core/Reflection.hpp"

#include "Render/VulkanRenderer.h"
#include "Render/VulkanDrawable.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner),
		mesh(nullptr), mat(nullptr), drawable(nullptr)
	{
	}

	MeshRenderer::~MeshRenderer()
	{
		if (core::IsValid(this->mesh))
			gameObject.world.meshes.DestroyNotify(this, this->mesh);
		if (core::IsValid(this->mat))
			gameObject.world.materials.DestroyNotify(this, this->mat);
	}

	void MeshRenderer::SetMesh(sh::render::Mesh& mesh)
	{
		if(core::IsValid(this->mesh))
			gameObject.world.meshes.DestroyNotify(this, this->mesh);
		this->mesh = &mesh;
	}

	auto MeshRenderer::GetMesh() const -> const sh::render::Mesh&
	{
		return *mesh;
	}

	void MeshRenderer::SetMaterial(sh::render::Material& mat)
	{
		if (core::IsValid(this->mat))
			gameObject.world.materials.DestroyNotify(this, this->mat);
		this->mat = &mat;
	}

	auto MeshRenderer::GetMaterial() const -> const sh::render::Material&
	{
		return *mat;
	}

	void MeshRenderer::CreateDrawable()
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

		if (gameObject.world.renderer.apiType == sh::render::RenderAPI::Vulkan)
		{
			render::VulkanRenderer& renderer = static_cast<render::VulkanRenderer&>(gameObject.world.renderer);
			drawable = std::make_unique<sh::render::VulkanDrawable>(renderer);
			drawable->Build(mesh, mat);
		}
		gameObject.world.meshes.RegisterDestroyNotify(this, mesh, [&]()
		{
			drawable.reset();
		});
		gameObject.world.materials.RegisterDestroyNotify(this, mat, [&]()
		{
			drawable.reset();
		});
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
		if (!sh::core::IsValid(mesh))
			return;
		if (!sh::core::IsValid(mat))
			return;
		if (!sh::core::IsValid(mat->GetShader()))
			return;
		if (drawable == nullptr)
			return;

		Camera* cam = gameObject.world.mainCamera;
		if (!sh::core::IsValid(cam))
			return;

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->apiType == sh::render::RenderAPI::Vulkan)
		{
			size_t size = mat->GetShader()->uniforms[0].back().offset + mat->GetShader()->uniforms[0].back().size;
			uniformCopyData.resize(size);
			for (const auto& uniform : mat->GetShader()->uniforms[0])
			{
				if (uniform.typeName == sh::core::reflection::GetTypeName<glm::mat4>())
				{
					auto& m = cam->GetProjMatrix();
					if (uniform.name == "proj")
						std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetProjMatrix()[0], sizeof(glm::mat4));
					else if (uniform.name == "view")
						std::memcpy(uniformCopyData.data() + uniform.offset, &cam->GetViewMatrix()[0], sizeof(glm::mat4));
					else if (uniform.name == "model")
						std::memcpy(uniformCopyData.data() + uniform.offset, &gameObject.transform->localToWorldMatrix[0], sizeof(glm::mat4));
				}
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec4>())
					std::memcpy(uniformCopyData.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec4));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec3>())
					std::memcpy(uniformCopyData.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec3));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec2>())
					std::memcpy(uniformCopyData.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec2));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<float>())
				{
					float value = mat->GetFloat(uniform.name);
					std::memcpy(uniformCopyData.data() + uniform.offset, &value, sizeof(float));
				}
			}

			int frameIdx = static_cast<sh::render::VulkanRenderer*>(renderer)->GetCurrentFrame();
			static_cast<sh::render::VulkanDrawable*>(drawable.get())->SetUniformData(frameIdx, uniformCopyData.data());
		}
		gameObject.world.renderer.PushDrawAble(drawable.get());
	}
}