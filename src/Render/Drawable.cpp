#include "Drawable.h"

#include "core/ThreadSyncManager.h"

#include <cassert>

namespace sh::render
{
	Drawable::Drawable(const Material& material, const Mesh& mesh) :
		mat(&material), mesh(&mesh), modelMatrix(1.0f)
	{
	}
	Drawable::Drawable(Drawable&& other) noexcept :
		mat(other.mat), mesh(other.mesh), modelMatrix(other.modelMatrix),
		materialData(std::move(other.materialData)), 
		light(other.light),
		renderTag(other.renderTag), 
		bDirty(other.bDirty)
	{
		other.bDirty = false;
	}
	Drawable::~Drawable()
	{
		SH_INFO_FORMAT("~Drawable: {}", (void*)this);
	}

	SH_RENDER_API void Drawable::Build(const IRenderContext& context)
	{
		this->context = &context;

		assert(core::IsValid(mat->GetShader()));
		if (!core::IsValid(mat->GetShader()))
			return;

		materialData.Create(context, *mat->GetShader(), true);
	}

	SH_RENDER_API void Drawable::SetMesh(const Mesh& mesh)
	{
		this->mesh = &mesh;
	}
	SH_RENDER_API void Drawable::SetMaterial(const Material& mat)
	{
		this->mat = &mat;
		if (core::IsValid(mat.GetShader()))
			materialData.Create(*context, *mat.GetShader(), true);
	}

	SH_RENDER_API auto Drawable::GetMaterial() const -> const Material*
	{
		return mat;
	}
	SH_RENDER_API auto Drawable::GetMesh() const -> const Mesh*
	{
		return mesh;
	}
	SH_RENDER_API auto Drawable::GetMaterialData() const -> const MaterialData&
	{
		return materialData;
	}
	SH_RENDER_API auto Drawable::GetMaterialData() -> MaterialData&
	{
		return materialData;
	}

	SH_RENDER_API void Drawable::SetModelMatrix(const glm::mat4& mat)
	{
		modelMatrix = mat;
	}
	SH_RENDER_API auto Drawable::GetModelMatrix() const -> const glm::mat4&
	{
		return modelMatrix;
	}

	SH_RENDER_API void Drawable::SetLightData(const Light& lightData)
	{
		light = lightData;
	}
	SH_RENDER_API auto Drawable::GetLightData() const -> const Light&
	{
		return light;
	}

	SH_RENDER_API auto Drawable::CheckAssetValid() const -> bool
	{
		return core::IsValid(mat) && core::IsValid(mesh);
	}
	SH_RENDER_API void Drawable::SetRenderTagId(uint32_t tagId)
	{
		renderTag = tagId;
	}
	SH_RENDER_API auto Drawable::GetRenderTagId() const -> uint32_t
	{
		return renderTag;
	}
}//namespace