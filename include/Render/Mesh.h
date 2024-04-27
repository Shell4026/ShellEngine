#pragma once

#include "Export.h"
#include "IDrawable.h"

#include <glm/glm.hpp>
#include <vector>
#include <initializer_list>

namespace sh::render
{
	class Mesh : public IDrawable
	{
	private:
		std::vector<glm::vec3> verts;
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
	};
}