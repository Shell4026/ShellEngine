#include "pch.h"
#include "Mesh.h"

#include "VertexBufferFactory.h"

namespace sh::render
{
	Mesh::Mesh() :
		attributes(attrs),
		topology(Topology::Face)
	{
		this->SetAttribute<glm::vec3>(ShaderAttribute<glm::vec3>{ "vertex" });
		this->SetAttribute<glm::vec2>(ShaderAttribute<glm::vec2>{ "uvs" });
		this->SetAttribute<glm::vec3>(ShaderAttribute<glm::vec3>{ "normals" });
	}
	Mesh::Mesh(const Mesh& other) :
		attributes(attrs),

		indices(other.indices), faces(other.faces),
		attrs(other.attrs.size()), buffer(),
		topology(other.topology), 
		bounding(other.bounding)
	{
		for (std::size_t i = 0; i < other.attrs.size(); ++i)
		{
			attrs[i] = other.attrs[i]->Clone();
		}
		buffer = other.buffer->Clone();
	}
	Mesh::Mesh(Mesh&& other) noexcept :
		attributes(attrs),

		indices(std::move(other.indices)), faces(std::move(other.faces)),
		attrs(std::move(other.attrs)), buffer(std::move(other.buffer)),
		topology(other.topology),
		bounding(std::move(other.bounding))
	{
	}
	Mesh::~Mesh()
	{
		SH_INFO("~?");
	}

	auto Mesh::operator=(const Mesh& other) -> Mesh&
	{
		indices = other.indices;
		faces = other.faces;

		for (std::size_t i = 0; i < other.attrs.size(); ++i)
		{
			attrs[i] = other.attrs[i]->Clone();
		}
		buffer = other.buffer->Clone();

		topology = other.topology;
		bounding = other.bounding;

		return *this;
	}
	auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
	{
		indices = std::move(other.indices);
		faces = std::move(other.faces);

		attrs = std::move(other.attrs);
		buffer = std::move(other.buffer);
		topology = other.topology;
		bounding = std::move(other.bounding);
		
		return *this;
	}

	void Mesh::SetVertex(const std::vector<glm::vec3>& verts)
	{
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[VERTEX_ID]).SetData(verts);
	}
	void Mesh::SetVertex(std::vector<glm::vec3>&& verts) noexcept
	{
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[VERTEX_ID]).SetData(verts);
	}
	void Mesh::SetVertex(const std::initializer_list<glm::vec3>& verts)
	{
		std::vector<glm::vec3> vec(verts.size());
		for (int i = 0; i < vec.size(); ++i)
			vec[i] = *(verts.begin() + i);
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[VERTEX_ID]).SetData(vec);
	}
	auto Mesh::GetVertex() const -> const std::vector<glm::vec3>&
	{
		return static_cast<ShaderAttribute<glm::vec3>&>(*attrs[VERTEX_ID]).GetDataT();
	}
	auto Mesh::GetVertexCount() const -> size_t
	{
		return static_cast<ShaderAttribute<glm::vec3>&>(*attrs[VERTEX_ID]).GetDataT().size();
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

	void Mesh::SetNormal(const std::vector<glm::vec3>& normals)
	{
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[NORMAL_ID]).SetData(normals);
	}
	void Mesh::SetNormal(std::vector<glm::vec3>&& normals) noexcept
	{
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[NORMAL_ID]).SetData(normals);
	}
	void Mesh::SetNormal(const std::initializer_list<glm::vec3>& normals)
	{
		static_cast<ShaderAttribute<glm::vec3>&>(*attrs[NORMAL_ID]).SetData(normals);
	}
	auto Mesh::GetNormal() const -> const std::vector<glm::vec3>&
	{
		return static_cast<ShaderAttribute<glm::vec3>&>(*attrs[NORMAL_ID]).GetDataT();
	}

	void Mesh::SetUV(const std::vector<glm::vec2>& uvs)
	{
		static_cast<ShaderAttribute<glm::vec2>&>(*attrs[UV_ID]).SetData(uvs);
	}
	void Mesh::SetUV(std::vector<glm::vec2>&& uvs) noexcept
	{
		static_cast<ShaderAttribute<glm::vec2>&>(*attrs[UV_ID]).SetData(uvs);
	}
	void Mesh::SetUV(const std::initializer_list<glm::vec2>& uvs)
	{
		static_cast<ShaderAttribute<glm::vec2>&>(*attrs[UV_ID]).SetData(uvs);
	}
	auto Mesh::GetUV() const -> const std::vector<glm::vec2>&
	{
		return static_cast<ShaderAttribute<glm::vec2>&>(*attrs[UV_ID]).GetDataT();
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

	auto Mesh::GetAttribute(std::string_view name) const -> const ShaderAttributeBase*
	{
		for (auto& attr : attrs)
		{
			if (attr->name == name)
			{
				return attr.get();
			}
		}
		return nullptr;
	}

	SH_RENDER_API void Mesh::ClearAttribute()
	{
		attrs.clear();
	}
}