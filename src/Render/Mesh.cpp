#include "pch.h"
#include "Mesh.h"

#include "VertexBufferFactory.h"

namespace sh::render
{
	Mesh::Mesh() :
		attributes(attrs),
		topology(Topology::Face)
	{

	}
	Mesh::Mesh(Mesh&& other) noexcept :
		attributes(attrs),
		attrs(std::move(other.attrs)),
		topology(other.topology)
	{
		verts = std::move(other.verts);
		indices = std::move(other.indices);
	}
	Mesh::~Mesh()
	{

	}

	auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
	{
		verts = std::move(other.verts);
		indices = std::move(other.indices);
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

	auto Mesh::GetVertex() const -> const std::vector<glm::vec3>&
	{
		return verts;
	}

	auto Mesh::GetVertexCount() const -> size_t
	{
		return verts.size();
	}

	void Mesh::SetIndices(const std::vector<uint32_t>& indices)
	{
		this->indices = indices;
	}

	void Mesh::SetIndices(std::vector<uint32_t>&& indices)
	{
		this->indices = std::move(indices);
	}

	void Mesh::SetIndices(const std::initializer_list<uint32_t>& indices)
	{
		this->indices.clear();
		this->indices.resize(indices.size());
		for (int i = 0; i < indices.size(); ++i)
		{
			this->indices[i] = *(indices.begin() + i);
		}
	}

	auto Mesh::GetIndices() const -> const std::vector<uint32_t>&
	{
		return indices;
	}

	auto Mesh::GetFaces() const -> const std::vector<Face>&
	{
		return faces;
	}

	void Mesh::Build(const Renderer& renderer)
	{
		if (topology == Topology::Face)
		{
			for (int i = 0; i < indices.size(); i += 3)
			{
				Face face;
				face.vertexIdx[0] = indices[i + 0];
				face.vertexIdx[1] = indices[i + 1];
				face.vertexIdx[2] = indices[i + 2];
				faces.push_back(face);
			}
		}
		
		buffer = VertexBufferFactory::Create(renderer, *this);
	}

	auto Mesh::GetVertexBuffer() const -> IVertexBuffer*
	{
		return buffer.get();
	}

	void Mesh::SetTopology(Topology topology)
	{
		this->topology = topology;
	}

	auto Mesh::GetTopology() const -> Topology
	{
		return topology;
	}

	auto Mesh::GetBoundingBox() const -> const Bounding&
	{
		return bounding;
	}
	auto Mesh::GetBoundingBox() -> Bounding&
	{
		return bounding;
	}
}