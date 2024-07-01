#pragma once

#include "Export.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"

#include <vector>

namespace sh::render
{
	class Mesh;
	class Material;
	class Texture;
	class Framebuffer;

	class IDrawable : public core::SObject, public core::INonCopyable
	{
	public:
		SH_RENDER_API virtual ~IDrawable() = default;
		SH_RENDER_API virtual void Build(Mesh* mesh, Material* mat) = 0;

		SH_RENDER_API virtual auto GetMaterial() const -> Material* = 0;
		SH_RENDER_API virtual auto GetMesh() const-> Mesh* = 0;

		SH_RENDER_API virtual void SetUniformData(uint32_t binding, const void* data) = 0;
		SH_RENDER_API virtual void SetTextureData(uint32_t binding, Texture* tex) = 0;

		SH_RENDER_API virtual void SetFramebuffer(Framebuffer& framebuffer) = 0;
		SH_RENDER_API virtual auto GetFramebuffer() const -> const Framebuffer* = 0;
	};
}