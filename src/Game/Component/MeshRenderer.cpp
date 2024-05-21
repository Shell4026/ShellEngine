#include "Component/MeshRenderer.h"

#include "Component/Camera.h"

#include "GameObject.h"

#include "Core/Util.h"
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

		ubo.proj = cam->GetProjMatrix();
		ubo.view = cam->GetViewMatrix();
		ubo.model = gameObject.transform->localToWorldMatrix;

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->apiType == sh::render::RenderAPI::Vulkan)
		{
			std::vector<unsigned char> data;
			
			size_t size = mat->GetShader()->uniforms[0].back().offset + mat->GetShader()->uniforms[0].back().size;
			data.resize(size);
			for (const auto& uniform : mat->GetShader()->uniforms[0])
			{
				if (uniform.typeName == sh::core::reflection::GetTypeName<glm::mat4>())
				{
					auto& m = cam->GetProjMatrix();
					if (uniform.name == "proj")
						std::memcpy(data.data() + uniform.offset, &cam->GetProjMatrix()[0], sizeof(glm::mat4));
					else if (uniform.name == "view")
						std::memcpy(data.data() + uniform.offset, &cam->GetViewMatrix()[0], sizeof(glm::mat4));
					else if (uniform.name == "model")
						std::memcpy(data.data() + uniform.offset, &gameObject.transform->localToWorldMatrix[0], sizeof(glm::mat4));
				}
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec4>())
					std::memcpy(data.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec4));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec3>())
					std::memcpy(data.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec3));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<glm::vec2>())
					std::memcpy(data.data() + uniform.offset, mat->GetVector(uniform.name), sizeof(glm::vec2));
				else if (uniform.typeName == sh::core::reflection::GetTypeName<float>())
				{
					float value = mat->GetFloat(uniform.name);
					std::memcpy(data.data() + uniform.offset, &value, sizeof(float));
				}
			}

			int frameIdx = static_cast<sh::render::VulkanRenderer*>(renderer)->GetCurrentFrame();
			static_cast<sh::render::VulkanDrawable*>(drawable)->SetUniformData(frameIdx, data.data());
		}
		gameObject.world.renderer.PushDrawAble(drawable);
	}
}