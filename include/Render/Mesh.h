#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "Renderer.h"
#include "ShaderAttribute.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <initializer_list>
#include <memory>

namespace sh::render
{
	class Material;

	class Mesh : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(Mesh)
	private:
		const Renderer& renderer;
		PROPERTY(mats)
		std::vector<Material*> mats;

		std::vector<glm::vec3> verts;
		std::vector<uint32_t> indices;

		std::vector<std::unique_ptr<ShaderAttributeBase>> attrs;

		std::unique_ptr<IDrawable> drawable;
	public:
		const std::vector< std::unique_ptr<ShaderAttributeBase>>& attributes;
	public:
		SH_RENDER_API Mesh(const Renderer& renderer);
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API ~Mesh();

		SH_RENDER_API auto operator=(Mesh&& other) noexcept -> Mesh&;

		SH_RENDER_API void SetVertex(const std::vector<glm::vec3>& verts);
		SH_RENDER_API void SetVertex(std::vector<glm::vec3>&& verts);
		SH_RENDER_API void SetVertex(const std::initializer_list<glm::vec3>& verts);
		SH_RENDER_API auto GetVertex() const -> const std::vector<glm::vec3>&;

		SH_RENDER_API auto GetVertexCount() const -> size_t;

		SH_RENDER_API void SetIndices(const std::vector<uint32_t >&indices);
		SH_RENDER_API void SetIndices(std::vector<uint32_t>&& indices);
		SH_RENDER_API void SetIndices(const std::initializer_list<uint32_t>& indices);
		SH_RENDER_API auto GetIndices() -> const std::vector<uint32_t>&;

		template<typename T>
		void SetAttribute(const ShaderAttribute<T>& attr);
		template<typename T>
		void SetAttribute(ShaderAttribute<T>&& attr);

		SH_RENDER_API void AddMaterial(Material* mat);
		SH_RENDER_API void SetMaterial(int id, Material* mat);
		SH_RENDER_API auto GetMaterial(int id) -> Material*;
		SH_RENDER_API auto GetMaterials() -> std::vector<Material*>&;

		SH_RENDER_API auto GetDrawable() const -> IDrawable*;
		SH_RENDER_API void Build();
	};

	template<typename T>
	inline void Mesh::SetAttribute(const ShaderAttribute<T>& attr)
	{
		attrs.push_back(std::make_unique<ShaderAttribute<T>>(attr));
	}
	template<typename T>
	inline void Mesh::SetAttribute(ShaderAttribute<T>&& attr)
	{
		attrs.push_back(std::make_unique<ShaderAttribute<T>>(std::move(attr)));
	}
}