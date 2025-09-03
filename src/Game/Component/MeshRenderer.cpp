#include "Component/MeshRenderer.h"
#include "Component/Camera.h"
#include "Component/PickingRenderer.h"
#include "Component/PointLight.h"
#include "Component/DirectionalLight.h"

#include "GameObject.h"

#include "Core/Reflection.hpp"

#include "Render/Drawable.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner),
		mesh(nullptr), mat(nullptr), drawable(nullptr)
	{
		onMatrixUpdateListener.SetCallback([&](const glm::mat4& mat)
			{
				if (core::IsValid(mesh))
					worldAABB = mesh->GetBoundingBox().GetWorldAABB(mat);
			}
		);
		gameObject.transform->onMatrixUpdate.Register(onMatrixUpdateListener);

		canPlayInEditor = true;
	}

	MeshRenderer::~MeshRenderer()
	{
	}

	void MeshRenderer::SetMesh(const render::Mesh* mesh)
	{
		this->mesh = mesh;
		if (core::IsValid(mesh))
		{
			worldAABB = mesh->GetBoundingBox().GetWorldAABB(gameObject.transform->localToWorldMatrix);

			if (drawable == nullptr)
				CreateDrawable();
			else
				drawable->SetMesh(*this->mesh);
		}
	}

	SH_GAME_API auto MeshRenderer::GetMesh() const -> const sh::render::Mesh*
	{
		return mesh;
	}

	void MeshRenderer::SetMaterial(sh::render::Material* mat)
	{
		if (core::IsValid(mat))
			this->mat = mat;
		else
			this->mat = gameObject.world.materials.GetResource("ErrorMaterial");

		if (drawable == nullptr)
			CreateDrawable();
		else
			drawable->SetMaterial(*this->mat);

		if (propertyBlock != nullptr) 
			SetMaterialPropertyBlock(propertyBlock); // 로컬 프로퍼티 처리를 위해 다시 호출

		auto shader = this->mat->GetShader();
		if (core::IsValid(shader))
		{
			for (auto& lightingPass : shader->GetAllShaderPass())
			{
				bool exit = false;
				for (auto& pass : lightingPass.passes)
				{
					if (pass->IsUsingLight())
					{
						bShaderHasLight = true;
						exit = true;
						break;
					}
				}
				if (exit)
					break;
			}
		}
	}

	auto MeshRenderer::GetMaterial() const -> sh::render::Material*
	{
		return mat;
	}

	void MeshRenderer::UpdateMaterialData()
	{
		if (drawable == nullptr || propertyBlock == nullptr || !core::IsValid(mat))
			return;
		render::Shader* shader = mat->GetShader();
		if (!core::IsValid(shader))
			return;
		
		for (auto& [passPtr, layoutPtr] : localUniformLocations)
		{
			std::vector<uint8_t> data(layoutPtr->GetSize());
			bool isSampler = false;
			for (auto& member : layoutPtr->GetMembers())
			{
				if (member.typeHash == core::reflection::GetType<int>().hash)
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(static_cast<int>(var.value()), data, member.offset);
					else
						SetData(0, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<float>().hash)
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(var.value(), data, member.offset);
					else
						SetData(0.0f, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec2>().hash)
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec2{ var->x, var->y }, data, member.offset);
					else
						SetData(glm::vec2{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec3>().hash)
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec3{ var->x, var->y, var->z }, data, member.offset);
					else
						SetData(glm::vec3{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::vec4>().hash)
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::vec4{ 0.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat2>().hash)
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat2(*var), data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat3>().hash)
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat3(*var), data, member.offset);
					else
						SetData(glm::mat3{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<glm::mat4>().hash)
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.typeHash == core::reflection::GetType<render::Texture>().hash)
				{
					auto var = propertyBlock->GetTextureProperty(member.name);
					if (var)
						drawable->GetMaterialData().SetTextureData(*passPtr, layoutPtr->type, layoutPtr->binding, var);

					isSampler = true;
				}
			}
			if (!isSampler)
				drawable->GetMaterialData().SetUniformData(*passPtr, layoutPtr->type, layoutPtr->binding, data.data());
		}
	}

	void MeshRenderer::FillLightStruct(render::Drawable& drawable) const
	{
		render::Drawable::Light lightStruct{};
		auto lights = world.GetLightOctree().Query(worldAABB);
		if (lights.size() < 10)
		{
			int idx = 0;
			for (int i = 0; i < lights.size(); ++i)
			{
				const ILight* light = static_cast<ILight*>(lights[i]);
				if (light->GetLightType() == ILight::Type::Point)
				{
					const PointLight* pointLight = static_cast<const PointLight*>(light);
					if (!core::IsValid(pointLight))
						continue;
					const Vec3& pos = pointLight->gameObject.transform->GetWorldPosition();
					lightStruct.lightPos[idx] = { pos.x, pos.y, pos.z, 0.f };
					lightStruct.lightPos[idx].w = pointLight->GetRadius();
					lightStruct.other[idx].w = 1;
				}
				else if (light->GetLightType() == ILight::Type::Directional)
				{
					const DirectionalLight* dirLight = static_cast<const DirectionalLight*>(light);
					if (!core::IsValid(dirLight))
						continue;
					const Vec3& dir = dirLight->GetDirection();
					lightStruct.lightPos[idx] = { dir.x, dir.y, dir.z, dirLight->GetIntensity() };
					lightStruct.other[idx].w = 0;
				}
				++idx;
			}
			lightStruct.lightCount = idx;
		}
		drawable.SetLightData(lightStruct);
	}

	void MeshRenderer::CreateDrawable()
	{
		if (!core::IsValid(mesh))
		{
			mesh = nullptr;
			return;
		}
		if (!core::IsValid(mat))
			mat = static_cast<render::Material*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{"bbc4ef7ec45dce223297a224f8093f10"}));
		if (!core::IsValid(mat->GetShader()))
			return;

		drawable = core::SObject::Create<render::Drawable>(*mat, *mesh);
		drawable->SetRenderTagId(renderTag);
		drawable->SetTopology(mesh->GetTopology());
		drawable->Build(*world.renderer.GetContext());
	}

	SH_GAME_API void MeshRenderer::UpdateDrawable()
	{
		if (bShaderHasLight)
			FillLightStruct(*drawable);

		drawable->SetModelMatrix(gameObject.transform->localToWorldMatrix);
	}

	SH_GAME_API void MeshRenderer::OnDestroy()
	{
		if (drawable != nullptr)
		{
			drawable->Destroy();
			drawable = nullptr;
		}
		Super::OnDestroy();
	}

	void MeshRenderer::Awake()
	{
		Super::Awake();

		if (!core::IsValid(mat))
			mat = gameObject.world.materials.GetResource("ErrorMaterial");
	}

	void MeshRenderer::Start()
	{
		if (drawable == nullptr)
			CreateDrawable();
	}

	void MeshRenderer::LateUpdate()
	{
		if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat) || !sh::core::IsValid(mat->GetShader()) || drawable == nullptr)
			return;

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->IsPause())
			return;

		mat->UpdateUniformBuffers();
		UpdateMaterialData();
		UpdateDrawable();

		gameObject.world.renderer.PushDrawAble(drawable);
	}

	SH_GAME_API void MeshRenderer::SetMaterialPropertyBlock(render::MaterialPropertyBlock* block)
	{
		propertyBlock = block;

		// 로컬 프로퍼티 위치 파악
		localUniformLocations.clear();

		render::Shader* shader = mat->GetShader();
		if (core::IsValid(shader))
		{
			for (auto& [propName, propInfo] : shader->GetProperties())
			{
				for (auto& location : propInfo.locations)
				{
					if (location.layoutPtr->type != render::UniformStructLayout::Type::Object)
						continue;
					auto it = std::find(localUniformLocations.begin(), localUniformLocations.end(), std::pair{ location.passPtr.Get(), location.layoutPtr});
					if (it == localUniformLocations.end())
						localUniformLocations.push_back({ location.passPtr.Get(), location.layoutPtr});
				}
			}
		}
	}

	SH_GAME_API auto MeshRenderer::GetMaterialPropertyBlock() const -> render::MaterialPropertyBlock*
	{
		return propertyBlock;
	}

	SH_GAME_API void MeshRenderer::SetRenderTagId(uint32_t tagId)
	{
		renderTag = tagId;
	}

	SH_GAME_API auto MeshRenderer::GetRenderTagId() const -> uint32_t
	{
		return renderTag;
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
		else if (prop.GetName() == "renderTag")
		{
			if (drawable != nullptr)
				drawable->SetRenderTagId(renderTag);
		}
	}
}//namespace