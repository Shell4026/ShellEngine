#include "Mesh.h"

#include "VulkanRenderer.h"
#include "VulkanVertexBuffer.h"

namespace sh::render
{
	Mesh::Mesh() :
		attributes(attrs)
	{

	}
	Mesh::Mesh(Mesh&& other) noexcept :
		attributes(attrs),
		attrs(std::move(other.attrs))
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

	void Mesh::Build(const Renderer& renderer)
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			buffer = std::make_unique<VulkanVertexBuffer>(static_cast<const VulkanRenderer&>(renderer));
			buffer->Create(*this);
		}
	}

	auto Mesh::GetVertexBuffer() const -> VertexBuffer*
	{
		return buffer.get();
	}
}