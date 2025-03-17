#include "Component/MeshRenderer.h"
#include "Component/Camera.h"
#include "Component/PickingRenderer.h"
#include "Component/PointLight.h"

#include "gameObject.h"

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
	SH_GAME_API auto MeshRenderer::GetMesh() -> sh::render::Mesh*
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
				if (member.type == core::reflection::GetType<int>())
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(static_cast<int>(var.value()), data, member.offset);
					else
						SetData(0, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<float>())
				{
					auto var = propertyBlock->GetScalarProperty(member.name);
					if (var.has_value())
						SetData(var.value(), data, member.offset);
					else
						SetData(0.0f, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec2>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec2{ var->x, var->y }, data, member.offset);
					else
						SetData(glm::vec2{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec3>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(glm::vec3{ var->x, var->y, var->z }, data, member.offset);
					else
						SetData(glm::vec3{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::vec4>())
				{
					auto var = propertyBlock->GetVectorProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::vec4{ 0.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat2>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat2(*var), data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat3>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(core::Util::ConvertMat4ToMat3(*var), data, member.offset);
					else
						SetData(glm::mat3{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<glm::mat4>())
				{
					auto var = propertyBlock->GetMatrixProperty(member.name);
					if (var)
						SetData(*var, data, member.offset);
					else
						SetData(glm::mat2{ 1.f }, data, member.offset);
				}
				else if (member.type == core::reflection::GetType<render::Texture>())
				{
					auto var = propertyBlock->GetTextureProperty(member.name);
					if (var)
						drawable->GetMaterialData().SetTextureData(*passPtr, layoutPtr->type, layoutPtr->binding, var);

					isSampler = true;
				}
			}
			if (!isSampler)
				drawable->GetMaterialData().SetUniformData(*passPtr, layoutPtr->type, layoutPtr->binding, data.data(), core::ThreadType::Game);
		}
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
		if (!core::IsValid(mat->GetShader()))
			return;

		drawable = core::SObject::Create<render::Drawable>(*mat, *mesh);
		drawable->SetRenderTagId(renderTag);
		drawable->Build(*world.renderer.GetContext());
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

	void MeshRenderer::Update()
	{
		if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat) || !sh::core::IsValid(mat->GetShader()))
			return;

		sh::render::Renderer* renderer = &gameObject.world.renderer;
		if (renderer->IsPause())
			return;

		mat->UpdateUniformBuffers();
		UpdateMaterialData();

		drawable->SetModelMatrix(gameObject.transform->localToWorldMatrix);
		gameObject.world.renderer.PushDrawAble(drawable);
		// 광원 구조체 채우기
		//struct
		//{
		//	alignas(16) glm::vec4 lightPosRange[10];
		//	alignas(16) int lightCount = 0;
		//} lightStruct;
		//if (bShaderHasLight)
		//{
		//	std::fill(lightStruct.lightPosRange, lightStruct.lightPosRange + 10, glm::vec4{ 0.f });
		//	auto lights = world.GetLightOctree().Query(worldAABB);
		//	if (lights.size() < 10)
		//	{
		//		int idx = 0;
		//		for (int i = 0; i < lights.size(); ++i)
		//		{
		//			ILight* light = static_cast<ILight*>(lights[i]);
		//			const Vec3& pos = light->gameObject.transform->GetWorldPosition();
		//			if (light->GetType() == PointLight::GetStaticType())
		//				lightStruct.lightPosRange[idx++] = { pos.x, pos.y, pos.z, static_cast<PointLight*>(light)->GetRadius() };
		//		}
		//		lightStruct.lightCount = idx;
		//	}
		//}
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
					auto it = std::find(localUniformLocations.begin(), localUniformLocations.end(), std::pair{ location.passPtr, location.layoutPtr });
					if (it == localUniformLocations.end())
						localUniformLocations.push_back({ location.passPtr, location.layoutPtr });
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