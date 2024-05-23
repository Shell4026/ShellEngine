﻿#pragma once

#include "Export.h"

#include "Core/SObject.h"

#include <vector>

namespace sh::render
{
	class Mesh;
	class Material;

	class IDrawable : public core::SObject
	{
	public:
		SH_RENDER_API virtual ~IDrawable() = default;
		SH_RENDER_API virtual void Build(Mesh* mesh, Material* mat) = 0;

		SH_RENDER_API virtual auto GetMaterial() const -> Material* = 0;
		SH_RENDER_API virtual auto GetMesh() const-> Mesh* = 0;
	};
}