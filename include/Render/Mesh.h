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

	struct SubMesh
	{
		std::size_t indexOffset = 0;
		std::size_t indexCount = 0;
	};
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
			glm::vec3 tangent;
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
		static constexpr uint8_t VERTEX_ID = 0;
		static constexpr uint8_t UV_ID = 1;
		static constexpr uint8_t NORMAL_ID = 2;
		static constexpr uint8_t TANGENT_ID = 3;
	public:
		SH_RENDER_API Mesh();
		SH_RENDER_API Mesh(const Mesh& other);
		SH_RENDER_API Mesh(Mesh&& other) noexcept;
		SH_RENDER_API virtual ~Mesh();

		SH_RENDER_API auto operator=(const Mesh& other) -> Mesh&;
		SH_RENDER_API auto operator=(Mesh&& other) noexcept -> Mesh&;

		SH_RENDER_API void SetVertex(std::vector<Vertex> verts) noexcept { this->verts = std::move(verts); }
		SH_RENDER_API void SetVertex(const std::initializer_list<Vertex>& verts);

		SH_RENDER_API void SetIndices(std::vector<uint32_t> indices) noexcept { this->indices = std::move(indices); }
		SH_RENDER_API void SetIndices(const std::initializer_list<uint32_t>& indices);

		SH_RENDER_API void SetSubMeshes(std::vector<SubMesh> subMeshes) { this->subMeshes = std::move(subMeshes); }

		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API void SetTopology(Topology topology) { this->topology = topology; }

		SH_RENDER_API void CalculateTangents();

		SH_RENDER_API auto GetVertex() const -> const std::vector<Vertex>& { return verts; }
		SH_RENDER_API auto GetVertexCount() const -> size_t { return verts.size(); }
		SH_RENDER_API auto GetFaces() const -> const std::vector<Face>& { return faces; }
		SH_RENDER_API auto GetIndices() const -> const std::vector<uint32_t>& { return indices; }
		SH_RENDER_API auto GetVertexBuffer() const -> IVertexBuffer* { return buffer.get(); }
		SH_RENDER_API auto GetTopology() const -> Topology { return topology; }
		SH_RENDER_API auto GetBoundingBox() const -> const AABB& { return bounding; }
		SH_RENDER_API auto GetBoundingBox() -> AABB& { return bounding; }
		SH_RENDER_API auto GetSubMeshes() const -> const std::vector<SubMesh>& { return subMeshes; }
	protected:
		SH_RENDER_API void SetVertexBuffer(std::unique_ptr<IVertexBuffer> buf);
		SH_RENDER_API void CreateFace();
	public:
		float lineWidth = 1.f;
	private:
		std::vector<Vertex> verts;
		std::vector<uint32_t> indices;
		std::vector<Face> faces;
		std::vector<SubMesh> subMeshes;

		std::unique_ptr<IVertexBuffer> buffer;

		AABB bounding;

		Topology topology;
	};//Mesh
}//namespace