#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "Renderer.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <initializer_list>

namespace sh::render
{
	class Material;

	class Mesh : public sh::core::SObject
	{
		SCLASS(Mesh)
	private:
		std::vector<glm::vec3> verts;
		PROPERTY(mats)
		std::vector<Material*> mats;
		std::unique_ptr<IDrawable> drawable;
	public:
		SH_RENDER_API Mesh();
		SH_RENDER_API Mesh(const Mesh& other);
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API ~Mesh();

		SH_RENDER_API auto operator=(const Mesh& other) -> Mesh&;
		SH_RENDER_API auto operator=(Mesh&& other) noexcept -> Mesh&;

		SH_RENDER_API void SetVertex(const std::vector<glm::vec3>& verts);
		SH_RENDER_API void SetVertex(std::vector<glm::vec3>&& verts);
		SH_RENDER_API void SetVertex(const std::initializer_list<glm::vec3>& verts);
		SH_RENDER_API auto GetVertex() -> std::vector<glm::vec3>&;
		SH_RENDER_API auto GetVertexConst() const -> const std::vector<glm::vec3>&;

		SH_RENDER_API auto GetVertexCount() const -> int;

		SH_RENDER_API void AddMaterial(Material* mat);
		SH_RENDER_API auto GetMaterial(int id) -> Material*;
		SH_RENDER_API auto GetMaterials() -> std::vector<Material*>&;

		SH_RENDER_API auto GetDrawable() const -> IDrawable*;
		SH_RENDER_API void Build(const Renderer& renderer);
	};
}