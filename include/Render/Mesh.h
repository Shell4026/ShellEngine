#pragma once

#include "Export.h"
#include "ShaderAttribute.h"
#include "AABB.h"
#include "IRenderResource.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/NonCopyable.h"

#include <glm/glm.hpp>
#include <vector>
#include <string_view>
#include <initializer_list>
#include <memory>
#include <array>

namespace sh::render
{
	class IVertexBuffer;

	/// @brief 모델 데이터를 지니는 클래스. 반드시 사용전 Build를 호출 해야한다.
	class Mesh : public core::SObject, public core::INonCopyable, public render::IRenderResource
	{
		SCLASS(Mesh)
	public:
		struct Face
		{
			std::array<uint32_t, 3> vertexIdx;
		};

		enum class Topology
		{
			Point,
			Line,
			Face
		};
	private:
		static constexpr uint8_t VERTEX_ID = 0;
		static constexpr uint8_t UV_ID = 1;
		static constexpr uint8_t NORMAL_ID = 2;

		std::vector<uint32_t> indices;
		std::vector<Face> faces;

		std::vector<std::unique_ptr<ShaderAttributeBase>> attrs;

		std::unique_ptr<IVertexBuffer> buffer;

		Topology topology;

		AABB bounding;
	public:
		const std::vector<std::unique_ptr<ShaderAttributeBase>>& attributes;

		float lineWidth = 1.f;
	public:
		SH_RENDER_API Mesh();
		SH_RENDER_API Mesh(const Mesh& other);
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API virtual ~Mesh();

		SH_RENDER_API auto operator=(const Mesh& other) -> Mesh&;
		SH_RENDER_API auto operator=(Mesh&& other) noexcept -> Mesh&;

		SH_RENDER_API void SetVertex(const std::vector<glm::vec3>& verts);
		SH_RENDER_API void SetVertex(std::vector<glm::vec3>&& verts) noexcept;
		SH_RENDER_API void SetVertex(const std::initializer_list<glm::vec3>& verts);
		SH_RENDER_API auto GetVertex() const -> const std::vector<glm::vec3>&;
		SH_RENDER_API auto GetVertexCount() const -> size_t;

		SH_RENDER_API void SetIndices(const std::vector<uint32_t >&indices);
		SH_RENDER_API void SetIndices(std::vector<uint32_t>&& indices);
		SH_RENDER_API void SetIndices(const std::initializer_list<uint32_t>& indices);
		SH_RENDER_API auto GetIndices() const -> const std::vector<uint32_t>&;

		SH_RENDER_API void SetNormal(const std::vector<glm::vec3>& normals);
		SH_RENDER_API void SetNormal(std::vector<glm::vec3>&& normals) noexcept;
		SH_RENDER_API void SetNormal(const std::initializer_list<glm::vec3>& normals);
		SH_RENDER_API auto GetNormal() const -> const std::vector<glm::vec3>&;

		SH_RENDER_API void SetUV(const std::vector<glm::vec2>& uvs);
		SH_RENDER_API void SetUV(std::vector<glm::vec2>&& uvs) noexcept;
		SH_RENDER_API void SetUV(const std::initializer_list<glm::vec2>& uvs);
		SH_RENDER_API auto GetUV() const -> const std::vector<glm::vec2>&;

		SH_RENDER_API auto GetFaces() const -> const std::vector<Face>&;

		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API auto GetVertexBuffer() const ->IVertexBuffer*;

		SH_RENDER_API void SetTopology(Topology topology);
		SH_RENDER_API auto GetTopology() const -> Topology;

		SH_RENDER_API auto GetBoundingBox() const -> const AABB&;
		SH_RENDER_API auto GetBoundingBox() -> AABB&;

		template<typename T>
		void SetAttribute(const ShaderAttribute<T>& attr);
		template<typename T>
		void SetAttribute(ShaderAttribute<T>&& attr);
		SH_RENDER_API auto GetAttribute(std::string_view name) const -> const ShaderAttributeBase*;
		SH_RENDER_API void ClearAttribute();
	};

	template<typename T>
	inline void Mesh::SetAttribute(const ShaderAttribute<T>& attr)
	{
		for (auto& attrPtr : attrs)
		{
			if (attrPtr->name == attr.name)
			{
				static_cast<ShaderAttribute<T>&>(*attrPtr.get()).SetData(attr.GetData());
				return;
			}
		}
		attrs.push_back(attr.Clone());
	}
	template<typename T>
	inline void Mesh::SetAttribute(ShaderAttribute<T>&& attr)
	{
		for (auto& attrPtr : attrs)
		{
			if (attrPtr->name == attr.name)
			{
				static_cast<ShaderAttribute<T>&>(*attrPtr.get()) = std::move(attr);
				return;
			}
		}
		attrs.push_back(std::make_unique<ShaderAttribute<T>>(std::move(attr)));
	}
}