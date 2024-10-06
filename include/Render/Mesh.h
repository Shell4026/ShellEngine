#pragma once

#include "Export.h"
#include "ShaderAttribute.h"
#include "Bounding.h"

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
	class Renderer;
	class IVertexBuffer;

	/// @brief 모델 데이터를 지니는 클래스. 반드시 사용전 Build를 호출 해야한다.
	class Mesh : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(Mesh)
	public:
		struct Face
		{
			std::array<uint32_t, 3> vertexIdx;
		};

		enum class Topology
		{
			Line,
			Face
		};
	private:
		std::vector<glm::vec3> verts;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;
		std::vector<uint32_t> indices;
		std::vector<Face> faces;

		std::vector<std::unique_ptr<ShaderAttributeBase>> attrs;

		std::unique_ptr<IVertexBuffer> buffer;

		Topology topology;

		Bounding bounding;
	public:
		const std::vector< std::unique_ptr<ShaderAttributeBase>>& attributes;
	public:
		SH_RENDER_API Mesh();
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API ~Mesh();

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

		SH_RENDER_API auto GetFaces() const -> const std::vector<Face>&;

		SH_RENDER_API void Build(const Renderer& renderer);

		SH_RENDER_API auto GetVertexBuffer() const ->IVertexBuffer*;

		SH_RENDER_API void SetTopology(Topology topology);
		SH_RENDER_API auto GetTopology() const -> Topology;

		SH_RENDER_API auto GetBoundingBox() const -> const Bounding&;
		SH_RENDER_API auto GetBoundingBox() -> Bounding&;

		template<typename T>
		void SetAttribute(const ShaderAttribute<T>& attr);
		template<typename T>
		void SetAttribute(ShaderAttribute<T>&& attr);
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