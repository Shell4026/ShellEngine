#include "Mesh.h"
#include "VulkanDrawable.h"

#include "VulkanRenderer.h"

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

	auto Mesh::GetIndices() -> const std::vector<uint32_t>&
	{
		return indices;
	}

	void Mesh::AddMaterial(Material* mat)
	{
		return mats.push_back(mat);
	}

	auto Mesh::GetMaterial(int id) -> Material*
	{
		return mats[id];
	}

	auto Mesh::GetMaterials() -> std::vector<Material*>&
	{
		return mats;
	}

	auto Mesh::GetDrawable() const -> IDrawable*
	{
		return drawable.get();
	}

	void Mesh::Build(const Renderer& renderer)
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			drawable = std::make_unique<VulkanDrawable>(static_cast<const VulkanRenderer&>(renderer));
			drawable->Build(GetMaterial(0), this);
		}
	}
}