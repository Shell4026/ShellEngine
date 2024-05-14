#include "Mesh.h"
#include "VulkanDrawable.h"

#include "VulkanRenderer.h"

namespace sh::render
{
	Mesh::Mesh(const Renderer& renderer) :
		renderer(renderer),
		attributes(attrs)
	{

	}
	Mesh::Mesh(Mesh&& other) noexcept :
		drawable(std::move(other.drawable)),
		mats(std::move(other.mats)),
		renderer(other.renderer),
		attrs(other.attrs),
		attributes(attrs)
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
		mats = std::move(other.mats);
		drawable = std::move(other.drawable);
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

	void Mesh::SetAttribute(std::string_view _name, const std::vector<glm::vec4>& attr)
	{
		std::string name{ _name };
		auto it = attrs.find(name);
		if (it == attrs.end())
			attrs.insert({ name, attr });
		else
			it->second = attr;
	}

	void Mesh::SetAttribute(std::string_view _name, std::vector<glm::vec4>&& attr)
	{
		std::string name{ _name };
		auto it = attrs.find(name);
		if (it == attrs.end())
			attrs.insert({ name, std::move(attr) });
		else
			it->second = std::move(attr);
	}

	void Mesh::SetAttribute(std::string_view _name, const std::initializer_list<glm::vec4>& _attr)
	{
		std::vector<glm::vec4> attr;
		attr.resize(_attr.size());
		for (int i = 0; i < attr.size(); ++i)
		{
			attr[i] = *(_attr.begin() + i);
		}

		std::string name{ _name };
		auto it = attrs.find(name);
		if (it == attrs.end())
			attrs.insert({ name, std::move(attr) });
		else
			it->second = std::move(attr);
	}

	void Mesh::SetMaterial(int id, Material* mat)
	{
		if (mats.size() < id + 1)
			mats.resize(id + 1);
		mats[id] = mat;

		Build();
	}

	void Mesh::AddMaterial(Material* mat)
	{
		mats.push_back(mat);
		Build();
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

	void Mesh::Build()
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			drawable = std::make_unique<VulkanDrawable>(static_cast<const VulkanRenderer&>(renderer));
			drawable->Build(GetMaterial(0), this);
		}
	}
}