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

	SH_RENDER_API auto Mesh::operator=(const Mesh& other) -> Mesh&
	{
		verts = other.verts;
		indices = other.indices;
		faces = other.faces;

		buffer = other.buffer->Clone();

		topology = other.topology;
		bounding = other.bounding;

		return *this;
	}
	SH_RENDER_API auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
	{
		indices = std::move(other.indices);
		faces = std::move(other.faces);

		verts = std::move(other.verts);
		buffer = std::move(other.buffer);
		topology = other.topology;
		bounding = std::move(other.bounding);
		
		return *this;
	}

	SH_RENDER_API void Mesh::SetVertex(const std::vector<Vertex>& verts)
	{
		this->verts = verts;
	}
	SH_RENDER_API void Mesh::SetVertex(const std::initializer_list<Vertex>& verts)
	{
		this->verts.clear();
		this->verts.reserve(verts.size());
		for (auto& vert : verts)
			this->verts.push_back(vert);
	}
	SH_RENDER_API auto Mesh::GetVertex() const -> const std::vector<Vertex>&
	{
		return verts;
	}
	SH_RENDER_API auto Mesh::GetVertexCount() const -> size_t
	{
		return verts.size();
	}

	SH_RENDER_API void Mesh::SetIndices(const std::vector<uint32_t>& indices)
	{
		this->indices = indices;
	}
	SH_RENDER_API void Mesh::SetIndices(std::vector<uint32_t>&& indices)
	{
		this->indices = std::move(indices);
	}
	SH_RENDER_API void Mesh::SetIndices(const std::initializer_list<uint32_t>& indices)
	{
		this->indices.resize(indices.size());
		for (int i = 0; i < indices.size(); ++i)
		{
			this->indices[i] = *(indices.begin() + i);
		}
	}
	SH_RENDER_API auto Mesh::GetIndices() const -> const std::vector<uint32_t>&
	{
		return indices;
	}

	SH_RENDER_API auto Mesh::GetFaces() const -> const std::vector<Face>&
	{
		return faces;
	}

	SH_RENDER_API void Mesh::Build(const IRenderContext& context)
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

	SH_RENDER_API auto Mesh::GetVertexBuffer() const -> IVertexBuffer*
	{
		return buffer.get();
	}

	SH_RENDER_API void Mesh::SetTopology(Topology topology)
	{
		this->topology = topology;
	}

	SH_RENDER_API auto Mesh::GetTopology() const -> Topology
	{
		return topology;
	}

	SH_RENDER_API auto Mesh::GetBoundingBox() const -> const AABB&
	{
		return bounding;
	}
	SH_RENDER_API auto Mesh::GetBoundingBox() -> AABB&
	{
		return bounding;
	}
	SH_RENDER_API void Mesh::CalculateTangents()
	{
		for (size_t i = 0; i < indices.size(); i += 3) 
		{
			uint32_t i0 = indices[i];
			uint32_t i1 = indices[i + 1];
			uint32_t i2 = indices[i + 2];

			Vertex& v0 = verts[i0];
			Vertex& v1 = verts[i1];
			Vertex& v2 = verts[i2];

			glm::vec3 edge1 = v1.vertex - v0.vertex;
			glm::vec3 edge2 = v2.vertex - v0.vertex;

			glm::vec2 deltaUV1 = v1.uv - v0.uv;
			glm::vec2 deltaUV2 = v2.uv - v0.uv;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + 1e-8f);

			glm::vec3 tangent;
			tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			tangent = glm::normalize(tangent);

			v0.tangent += tangent;
			v1.tangent += tangent;
			v2.tangent += tangent;
		}

		for (auto& v : verts)
			v.tangent = glm::normalize(v.tangent);
	}
}