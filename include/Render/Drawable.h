﻿#pragma once
#include "Export.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialData.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"

#include <vector>

namespace sh::render
{
	class Drawable : 
		public core::SObject, 
		public core::INonCopyable,
		public IRenderResource
	{
		SCLASS(Drawable)
	private:
		const IRenderContext* context = nullptr;

		PROPERTY(mat)
		const Material* mat = nullptr;
		PROPERTY(mesh)
		const Mesh* mesh = nullptr;

		glm::mat4 modelMatrix;

		MaterialData materialData;

		uint32_t renderTag = 1;

		bool bDirty = false;
	public:
		SH_RENDER_API Drawable(const Material& material, const Mesh& mesh);
		SH_RENDER_API virtual ~Drawable() = default;

		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API void SetMesh(const Mesh& mesh);
		SH_RENDER_API void SetMaterial(const Material& mat);

		SH_RENDER_API auto GetMesh() const -> const Mesh*;
		SH_RENDER_API auto GetMaterial() const -> const Material*;
		SH_RENDER_API auto GetMaterialData() const -> const MaterialData&;
		SH_RENDER_API auto GetMaterialData() -> MaterialData&;

		SH_RENDER_API void SetModelMatrix(const glm::mat4& mat);
		SH_RENDER_API auto GetModelMatrix() const -> const glm::mat4&;

		SH_RENDER_API auto CheckAssetValid() const -> bool;

		SH_RENDER_API void SetRenderTagId(uint32_t tagId);
		SH_RENDER_API auto GetRenderTagId() const -> uint32_t;
	};
}