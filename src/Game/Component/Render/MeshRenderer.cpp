#include "Component/Render/MeshRenderer.h"
#include "Component/Render/PointLight.h"
#include "Component/Render/DirectionalLight.h"

#include "World.h"

#include "Render/Renderer.h"

#include <cstring>
#include <algorithm>

namespace sh::game
{
	static core::UUID errorMatUUID{ "bbc4ef7ec45dce223297a224f8093f10" };
	static core::UUID errorShaderUUID{ "bbc4ef7ec45dce223297a224f8093f0f" };
	static core::UUID blackTexUUID{ "bbc4ef7ec45dce223297a224f8093f18" };

	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner),
		mesh(nullptr)
	{
		onMatrixUpdateListener.SetCallback(
			[this](const glm::mat4& mat)
			{
				if (core::IsValid(mesh))
					worldAABB = mesh->GetBoundingBox().GetWorldAABB(mat);
			}
		);
		gameObject.transform->onMatrixUpdate.Register(onMatrixUpdateListener);
		onShaderChangedListener.SetCallback(
			[this](const render::Shader* shader)
			{
				SH_INFO("Shader was changed");
				if (!core::IsValid(shader))
					return;
				SearchLocalProperties();
				SetDefaultLocalProperties();
			}
		);

		canPlayInEditor = true;
	}

	MeshRenderer::~MeshRenderer() = default;

	SH_GAME_API void MeshRenderer::OnDestroy()
	{
		for (render::Drawable* const drawable : drawables)
		{
			if (drawable != nullptr)
				drawable->Destroy();
		}
		drawables.clear();
		Super::OnDestroy();
	}
	SH_GAME_API void MeshRenderer::Awake()
	{
		Super::Awake();

		render::Material* const errorMat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(errorMatUUID));
		if (mats.empty() || !core::IsValid(mats[0]))
		{
			if (mats.empty())
				mats.push_back(nullptr);
			mats[0] = errorMat;
		}
	}
	SH_GAME_API void MeshRenderer::Start()
	{
		if (drawables.empty())
			CreateDrawable(true);
	}
	SH_GAME_API void MeshRenderer::LateUpdate()
	{
		if (!sh::core::IsValid(mesh) || mats.empty() || drawables.empty())
			return;

		sh::render::Renderer* const renderer = &gameObject.world.renderer;
		if (renderer->IsPause())
			return;

		for (std::size_t i = 0; i < drawables.size(); ++i)
		{
			render::Material* const mat = GetMaterial(i);
			if (!core::IsValid(mat) || !core::IsValid(mat->GetShader()))
				continue;
			mat->UpdateUniformBuffers();
		}
		UpdateDrawable();

		for (render::Drawable* const drawable : drawables)
			gameObject.world.renderer.PushDrawAble(drawable);
	}
	SH_GAME_API void MeshRenderer::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		if (json.contains("MeshRenderer") && json["MeshRenderer"].contains("mat")) // 레거시
		{
			const std::string& matUUIDStr = json["MeshRenderer"]["mat"].get_ref<const std::string&>();
			render::Material* const mat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(core::UUID{ matUUIDStr }));
			if (core::IsValid(mat))
				SetMaterial(mat);
		}
	}
	SH_GAME_API void MeshRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("mesh"))
		{
			SetMesh(mesh);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("mats"))
		{
			for (std::size_t i = 0; i < mats.size(); ++i)
				SetMaterial(i, mats[i]);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("renderTag"))
		{
			for (render::Drawable* drawable : drawables)
			{
				if (drawable != nullptr)
					drawable->SetRenderTagId(renderTag);
			}
		}
	}

	SH_GAME_API void MeshRenderer::SetMesh(const render::Mesh* mesh)
	{
		this->mesh = mesh;
		if (core::IsValid(mesh))
		{
			worldAABB = mesh->GetBoundingBox().GetWorldAABB(gameObject.transform->localToWorldMatrix);
			CreateDrawable(true);
		}
	}

	SH_GAME_API void MeshRenderer::SetMaterial(sh::render::Material* mat)
	{
		SetMaterial(0, mat);
	}

	SH_GAME_API void MeshRenderer::SetMaterial(std::size_t index, sh::render::Material* mat)
	{
		if (mats.size() <= index)
			mats.resize(index + 1, nullptr);

		if (core::IsValid(mats[index]))
			mats[index]->onShaderChanged.UnRegister(onShaderChangedListener);

		render::Material* const errorMat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(errorMatUUID));
		mats[index] = core::IsValid(mat) ? mat : errorMat;

		if (!core::IsValid(mats[index]->GetShader()))
			mats[index]->SetShader(static_cast<render::Shader*>(core::SObject::GetSObjectUsingResolver(errorShaderUUID)));

		mats[index]->onShaderChanged.Register(onShaderChangedListener);

		render::Shader* const shader = mats[index]->GetShader();
		if (core::IsValid(shader))
		{
			if (shader->IsUsingLight() && index < drawables.size() && drawables[index] != nullptr)
				FillLightStruct(*drawables[index], *shader);

			SearchLocalProperties();
		}

		if (index < drawables.size() && drawables[index] != nullptr)
			drawables[index]->SetMaterial(*mats[index]);
		else
			CreateDrawable(true);

		if (core::IsValid(shader))
			SetDefaultLocalProperties();
	}

	SH_GAME_API auto MeshRenderer::GetMaterial(std::size_t index) const -> sh::render::Material*
	{
		if (index < mats.size())
			return mats[index];
		if (!mats.empty())
			return mats.back();
		return nullptr;
	}

	SH_GAME_API auto MeshRenderer::GetMaterialCount() const -> std::size_t
	{
		return mats.size();
	}

	SH_GAME_API void MeshRenderer::SetMaterialPropertyBlock(std::unique_ptr<render::MaterialPropertyBlock>&& block)
	{
		SetMaterialPropertyBlock(0, std::move(block));
	}

	SH_GAME_API void MeshRenderer::SetMaterialPropertyBlock(std::size_t index, std::unique_ptr<render::MaterialPropertyBlock>&& block)
	{
		if (propertyBlocks.size() <= index)
			propertyBlocks.resize(index + 1);
		propertyBlocks[index] = std::move(block);
		SetDefaultLocalProperties();
	}

	SH_GAME_API auto MeshRenderer::GetMaterialPropertyBlock(std::size_t index) const -> render::MaterialPropertyBlock*
	{
		if (index < propertyBlocks.size())
			return propertyBlocks[index].get();
		return nullptr;
	}

	SH_GAME_API void MeshRenderer::UpdatePropertyBlockData()
	{
		if (drawables.empty() || propertyBlocks.empty())
			return;

		for (std::size_t i = 0; i < drawables.size(); ++i)
		{
			render::Drawable* const drawable = drawables[i];
			if (drawable == nullptr)
				continue;
			if (i >= propertyBlocks.size() || propertyBlocks[i] == nullptr)
				continue;
			if (i >= localUniformLocationsList.size())
				continue;

			auto* mat = GetMaterial(i);
			if (!core::IsValid(mat) || !core::IsValid(mat->GetShader()))
				continue;

			render::MaterialPropertyBlock& block = *propertyBlocks[i];
			auto& locations = localUniformLocationsList[i];

			for (auto& [passPtr, layoutPtr] : locations)
			{
				if (layoutPtr->usage == render::UniformStructLayout::Usage::Material)
					continue;

				std::vector<uint8_t> data(layoutPtr->GetSize());
				bool isSampler = false;
				for (auto& member : layoutPtr->GetMembers())
				{
					if (member.typeHash == core::reflection::GetType<int>().hash)
					{
						auto var = block.GetScalarProperty(member.name);
						SetData(var.has_value() ? static_cast<int>(var.value()) : 0, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<float>().hash)
					{
						auto var = block.GetScalarProperty(member.name);
						SetData(var.has_value() ? var.value() : 0.0f, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::vec2>().hash)
					{
						auto var = block.GetVectorProperty(member.name);
						SetData(var ? glm::vec2{ var->x, var->y } : glm::vec2{ 0.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::vec3>().hash)
					{
						auto var = block.GetVectorProperty(member.name);
						SetData(var ? glm::vec3{ var->x, var->y, var->z } : glm::vec3{ 0.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::vec4>().hash)
					{
						auto var = block.GetVectorProperty(member.name);
						SetData(var ? *var : glm::vec4{ 0.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::mat2>().hash)
					{
						auto var = block.GetMatrixProperty(member.name);
						SetData(var ? core::Util::ConvertMat4ToMat2(*var) : glm::mat2{ 1.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::mat3>().hash)
					{
						auto var = block.GetMatrixProperty(member.name);
						SetData(var ? core::Util::ConvertMat4ToMat3(*var) : glm::mat3{ 1.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<glm::mat4>().hash)
					{
						auto var = block.GetMatrixProperty(member.name);
						SetData(var ? *var : glm::mat4{ 1.f }, data, member.offset);
					}
					else if (member.typeHash == core::reflection::GetType<render::Texture>().hash)
					{
						auto var = block.GetTextureProperty(member.name);
						if (core::IsValid(var))
							drawable->GetMaterialData().SetTextureData(*passPtr, layoutPtr->usage, layoutPtr->binding, *var);
						else
						{
							render::Texture* const texPtr = static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(blackTexUUID));
							if (texPtr != nullptr)
								drawable->GetMaterialData().SetTextureData(*passPtr, layoutPtr->usage, layoutPtr->binding, *texPtr);
							else
								SH_ERROR("Can't get default texture!");
						}
						isSampler = true;
					}
				}
				if (!isSampler)
					drawable->GetMaterialData().SetBindingData(*passPtr, layoutPtr->usage, layoutPtr->binding, std::move(data));
			}
		}
	}
	SH_GAME_API void MeshRenderer::SetRenderTagId(uint32_t tagId)
	{
		renderTag = tagId;
	}
	SH_GAME_API void MeshRenderer::CreateDrawable(bool bUseSubMesh)
	{
		if (!core::IsValid(mesh))
		{
			mesh = nullptr;
			return;
		}
		render::Material* const errorMat = static_cast<render::Material*>(core::SObject::GetSObjectUsingResolver(errorMatUUID));

		for (render::Drawable* drawable : drawables)
		{
			if (drawable != nullptr)
				drawable->Destroy();
		}
		drawables.clear();

		const auto& subMeshes = mesh->GetSubMeshes();
		const std::size_t count = subMeshes.empty() || !bUseSubMesh ? 1 : subMeshes.size();

		while (mats.size() < count)
			mats.push_back(errorMat);

		propertyBlocks.resize(count);
		localUniformLocationsList.resize(count);

		for (std::size_t i = 0; i < count; ++i)
		{
			render::Material* const mat = core::IsValid(mats[i]) ? mats[i] : errorMat;
			if (!core::IsValid(mat->GetShader()))
				continue;

			render::Drawable* const drawable = core::SObject::Create<render::Drawable>(*mat, *mesh);
			if (!subMeshes.empty() && bUseSubMesh)
				drawable->SetSubMeshIndex(static_cast<int>(i));
			drawable->SetRenderTagId(renderTag);
			drawable->SetTopology(mesh->GetTopology());
			drawable->Build(*world.renderer.GetContext());
			drawables.push_back(drawable);
		}

		UpdatePropertyBlockData();
	}
	SH_GAME_API void MeshRenderer::UpdateDrawable()
	{
		for (std::size_t i = 0; i < drawables.size(); ++i)
		{
			render::Drawable* const drawable = drawables[i];
			if (drawable == nullptr)
				continue;
			render::Material* const mat = GetMaterial(i);
			if (!core::IsValid(mat) || !core::IsValid(mat->GetShader()))
				continue;

			if (mat->GetShader()->IsUsingLight())
				FillLightStruct(*drawable, *mat->GetShader());

			drawable->SetModelMatrix(gameObject.transform->localToWorldMatrix);
		}
	}

	void MeshRenderer::SearchLocalProperties()
	{
		localUniformLocationsList.resize(mats.size());

		for (std::size_t i = 0; i < mats.size(); ++i)
		{
			localUniformLocationsList[i].clear();

			if (!core::IsValid(mats[i]))
				continue;
			render::Shader* const shader = mats[i]->GetShader();
			if (!core::IsValid(shader))
				continue;

			for (auto& [propName, propInfo] : shader->GetProperties())
			{
				for (auto& location : propInfo.locations)
				{
					if (location.layoutPtr->usage != render::UniformStructLayout::Usage::Object)
						continue;
					auto& locs = localUniformLocationsList[i];
					auto it = std::find(locs.begin(), locs.end(), std::pair{ location.passPtr.Get(), location.layoutPtr });
					if (it == locs.end())
						locs.push_back({ location.passPtr.Get(), location.layoutPtr });
				}
			}
		}
	}

	void MeshRenderer::SetDefaultLocalProperties()
	{
		propertyBlocks.resize(mats.size());

		for (std::size_t i = 0; i < mats.size(); ++i)
		{
			if (i >= localUniformLocationsList.size() || localUniformLocationsList[i].empty())
				continue;
			if (!core::IsValid(mats[i]))
				continue;
			render::Shader* const shader = mats[i]->GetShader();
			if (!core::IsValid(shader))
				continue;

			if (propertyBlocks[i] == nullptr)
				propertyBlocks[i] = std::make_unique<render::MaterialPropertyBlock>();

			render::MaterialPropertyBlock& block = *propertyBlocks[i];
			for (const auto& [propName, propInfo] : shader->GetProperties())
			{
				if (!propInfo.bLocalProperty)
					continue;

				if (propInfo.type == core::reflection::GetType<int>() || propInfo.type == core::reflection::GetType<float>())
					block.SetProperty(propName, 0.f);
				else if (propInfo.type == core::reflection::GetType<glm::vec4>())
					block.SetProperty(propName, glm::vec4{ 0.f });
				else if (propInfo.type == core::reflection::GetType<glm::vec3>())
					block.SetProperty(propName, glm::vec3{ 0.f });
				else if (propInfo.type == core::reflection::GetType<glm::vec2>())
					block.SetProperty(propName, glm::vec2{ 0.f });
				else if (propInfo.type == core::reflection::GetType<render::Texture>())
					block.SetProperty(propName, static_cast<render::Texture*>(core::SObject::GetSObjectUsingResolver(blackTexUUID)));
			}
		}
		UpdatePropertyBlockData();
	}
	void MeshRenderer::FillLightStruct(render::Drawable& drawable, render::Shader& shader)
	{
		const std::vector<game::IOctreeElement*>& lights = world.GetLightOctree().Query(worldAABB);

		lightDatas.clear();
		lightDatas.resize(sizeof(int) * 4 + lights.size() * sizeof(Light), 0); // int 3개는 패딩
		const int count = static_cast<int>(lights.size());
		std::memcpy(lightDatas.data(), &count, sizeof(int));
		std::size_t offset = sizeof(int) * 4;
		for (int i = 0; i < count; ++i)
		{
			Light lightStruct{};
			const ILight* light = static_cast<ILight*>(lights[i]);
			if (light->GetLightType() == ILight::Type::Point)
			{
				const PointLight* pointLight = static_cast<const PointLight*>(light);
				if (!core::IsValid(pointLight))
					continue;
				const Vec3& pos = pointLight->gameObject.transform->GetWorldPosition();
				lightStruct.pos = { pos.x, pos.y, pos.z, pointLight->GetRadius() };
				lightStruct.other.w = 1;
			}
			else if (light->GetLightType() == ILight::Type::Directional)
			{
				const DirectionalLight* dirLight = static_cast<const DirectionalLight*>(light);
				if (!core::IsValid(dirLight))
					continue;
				const Vec3& dir = dirLight->GetDirection();
				lightStruct.pos = { dir.x, dir.y, dir.z, dirLight->GetIntensity() };
				lightStruct.other.w = 0;
			}
			std::memcpy(lightDatas.data() + offset, &lightStruct, sizeof(Light));
			offset += sizeof(Light);
		}
		for (const render::Shader::LightingPassData& lightingPassData : shader.GetAllShaderPass())
		{
			for (const render::ShaderPass& pass : lightingPassData.passes)
			{
				if (pass.IsPendingKill() || pass.GetLightingBinding() == -1)
					continue;

				drawable.GetMaterialData().SetBindingData(pass, render::UniformStructLayout::Usage::Object, pass.GetLightingBinding(), lightDatas.data(), lightDatas.size());
			}
		}
	}
}//namespace