#pragma once

#include "Export.h"
#include "IDrawable.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <initializer_list>

namespace sh::render
{
	class Mesh : public sh::core::SObject, public IDrawable
	{
		SCLASS(Mesh)
	private:
		std::vector<glm::vec3> verts;
		std::vector<Material*> mats;
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

		SH_RENDER_API auto GetVertexCount() const -> int override;

		SH_RENDER_API void AddMaterial(Material* mat);
		SH_RENDER_API auto GetMaterial(int id) -> Material* override;
		SH_RENDER_API auto GetMaterials() -> std::vector<Material*>& override;
	};
}