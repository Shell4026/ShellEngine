#include "Mesh.h"

namespace sh::render
{
	Mesh::Mesh()
	{

	}
	Mesh::Mesh(const Mesh& other)
	{
		verts = other.verts;
	}
	Mesh::Mesh(Mesh&& other) noexcept
	{
		verts = std::move(other.verts);
	}
	Mesh::~Mesh()
	{

	}

	auto Mesh::operator=(const Mesh& other)->Mesh&
	{
		verts = other.verts;

		return *this;
	}
	auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
	{
		verts = std::move(other.verts);

		return *this;
	}

	void Mesh::SetVertex(const std::vector<glm::vec3>& verts)
	{
		this->verts = verts;
	}
	void Mesh::SetVertex(std::vector<glm::vec3>&& verts)
	{
		this->verts = std::move(verts);
	}
	void Mesh::SetVertex(const std::initializer_list<glm::vec3>& verts)
	{
		this->verts.clear();
		this->verts.resize(verts.size());
		for (int i = 0; i < verts.size(); ++i)
		{
			this->verts[i] = *(verts.begin() + i);
		}
	}
	auto Mesh::GetVertex() -> std::vector<glm::vec3>&
	{
		return verts;
	}

	auto Mesh::GetVertexConst() const -> const std::vector<glm::vec3>&
	{
		return verts;
	}

	auto Mesh::GetVertexCount() const -> int
	{
		return verts.size();
	}
}