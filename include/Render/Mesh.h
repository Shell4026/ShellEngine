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
		struct Vertex
		{
			glm::vec3 vertex;
			glm::vec2 uv;
			glm::vec3 normal;
		};
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
		std::vector<Vertex> verts;
		std::vector<uint32_t> indices;
		std::vector<Face> faces;

		std::unique_ptr<IVertexBuffer> buffer;

		Topology topology;

		AABB bounding;
	public:
		static constexpr uint8_t VERTEX_ID = 0;
		static constexpr uint8_t UV_ID = 1;
		static constexpr uint8_t NORMAL_ID = 2;

		float lineWidth = 1.f;
	public:
		SH_RENDER_API Mesh();
		SH_RENDER_API Mesh(const Mesh& other);
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API virtual ~Mesh();

		SH_RENDER_API auto operator=(const Mesh& other) -> Mesh&;
		SH_RENDER_API auto operator=(Mesh&& other) noexcept -> Mesh&;

		SH_RENDER_API void SetVertex(const std::vector<Vertex>& verts);
		SH_RENDER_API void SetVertex(const std::initializer_list<Vertex>& verts);
		SH_RENDER_API auto GetVertex() const -> const std::vector<Vertex>&;
		SH_RENDER_API auto GetVertexCount() const -> size_t;

		SH_RENDER_API void SetIndices(const std::vector<uint32_t >&indices);
		SH_RENDER_API void SetIndices(std::vector<uint32_t>&& indices);
		SH_RENDER_API void SetIndices(const std::initializer_list<uint32_t>& indices);
		SH_RENDER_API auto GetIndices() const -> const std::vector<uint32_t>&;

		SH_RENDER_API auto GetFaces() const -> const std::vector<Face>&;

		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API auto GetVertexBuffer() const ->IVertexBuffer*;

		SH_RENDER_API void SetTopology(Topology topology);
		SH_RENDER_API auto GetTopology() const -> Topology;

		SH_RENDER_API auto GetBoundingBox() const -> const AABB&;
		SH_RENDER_API auto GetBoundingBox() -> AABB&;
	};
}