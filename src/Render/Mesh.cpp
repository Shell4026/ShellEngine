#include "Mesh.h"

#include "VertexBufferFactory.h"

namespace sh::render
{
	Mesh::Mesh() :
		topology(Topology::Face)
	{
	}
	Mesh::Mesh(const Mesh& other) :
		verts(other.verts), indices(other.indices), faces(other.faces),
		topology(other.topology), 
		bounding(other.bounding)
	{
		buffer = other.buffer->Clone();
	}
	Mesh::Mesh(Mesh&& other) noexcept :
		verts(std::move(other.verts)), indices(std::move(other.indices)), faces(std::move(other.faces)),
		buffer(std::move(other.buffer)),
		topology(other.topology),
		bounding(std::move(other.bounding))
	{
	}
	Mesh::~Mesh()
	{
		SH_INFO("~Mesh");
	}

	auto Mesh::operator=(const Mesh& other) -> Mesh&
	{
		verts = other.verts;
		indices = other.indices;
		faces = other.faces;

		buffer = other.buffer->Clone();

		topology = other.topology;
		bounding = other.bounding;

		return *this;
	}
	auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
	{
		indices = std::move(other.indices);
		faces = std::move(other.faces);

		verts = std::move(other.verts);
		buffer = std::move(other.buffer);
		topology = other.topology;
		bounding = std::move(other.bounding);
		
		return *this;
	}

	void Mesh::SetVertex(const std::vector<Vertex>& verts)
	{
		this->verts = verts;
	}
	void Mesh::SetVertex(const std::initializer_list<Vertex>& verts)
	{
		this->verts.clear();
		this->verts.reserve(verts.size());
		for (auto& vert : verts)
			this->verts.push_back(vert);
	}
	auto Mesh::GetVertex() const -> const std::vector<Vertex>&
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

	void Mesh::Build(const IRenderContext& context)
	{
		if (topology == Topology::Face)
		{
			for (int i = 0; i < indices.size(); i += 3)
			{
				Face face{};
				face.vertexIdx[0] = indices[i + 0];
				face.vertexIdx[1] = indices[i + 1];
				face.vertexIdx[2] = indices[i + 2];
				faces.push_back(face);
			}
		}

		buffer = VertexBufferFactory::Create(context, *this);
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

	auto Mesh::GetBoundingBox() const -> const AABB&
	{
		return bounding;
	}
	auto Mesh::GetBoundingBox() -> AABB&
	{
		return bounding;
	}
}