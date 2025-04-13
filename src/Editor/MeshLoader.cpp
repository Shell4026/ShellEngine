#include "MeshLoader.h"

#include "Core/SObject.h"
#include "Core/Logger.h"

#include "Render/Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "External/tinyobjloader/tiny_obj_loader.h"

#include <fmt/core.h>

#include <string>
#include <cstdint>
namespace sh::editor
{
	SH_EDITOR_API auto MeshLoader::CalculateTangent(
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
		const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) const -> glm::vec3
	{
		glm::vec3 e0 = v1 - v0;
		glm::vec3 e1 = v2 - v0;
		glm::vec2 deltaUV0 = uv1 - uv0;
		glm::vec2 deltaUV1 = uv2 - uv0;

		float f = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y);

		float x = f * (deltaUV1.y * e0.x - deltaUV0.y * e1.x);
		float y = f * (deltaUV1.y * e0.y - deltaUV0.y * e1.y);
		float z = f * (deltaUV1.y * e0.z - deltaUV0.y * e1.z);
		return glm::vec3{ x, y, z };
	}
	MeshLoader::MeshLoader(const render::IRenderContext& context) :
		context(context)
	{
	}

	SH_EDITOR_API auto MeshLoader::Load(std::string_view filename) -> render::Mesh*
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		auto mesh = core::SObject::Create<render::Mesh>();
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.data()))
		{
			SH_ERROR_FORMAT("Can't load {}!", filename);
			return nullptr;
		}

		std::unordered_map<Indices, uint32_t> uniqueVerts;
		std::vector<render::Mesh::Vertex> verts;
		std::vector<uint32_t> indices;
		
		glm::vec3 min{}, max{};
		for (const auto& shape : shapes)
		{
			uint32_t n = 0;
			for (const auto& index : shape.mesh.indices) 
			{
				glm::vec3 vert
				{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				glm::vec3 normal
				{
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				glm::vec2 uv
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1 - attrib.texcoords[2 * index.texcoord_index + 1],
				};

				if (vert.x < min.x) min.x = vert.x;
				if (vert.y < min.y) min.y = vert.y;
				if (vert.z < min.z) min.z = vert.z;
				if (vert.x > max.x) max.x = vert.x;
				if (vert.y > max.y) max.y = vert.y;
				if (vert.z > max.z) max.z = vert.z;

				Indices indexList{ index.vertex_index, index.normal_index, index.texcoord_index };

				// 버텍스 위치가 겹치더라도 노말이나 UV도 겹치는지 비교해야 인덱스로 재사용 할 수 있다.
				auto it = uniqueVerts.find(indexList);
				if (it == uniqueVerts.end())
				{
					uniqueVerts.insert({ indexList, n });

					verts.push_back({ vert, uv, normal });
					indices.push_back(n++);
				}
				else
					indices.push_back(it->second);
			}
		}

		for (int i = 0; i < indices.size(); i += 3)
		{
			const glm::vec3& v0 = verts[indices[i + 0]].vertex;
			const glm::vec3& v1 = verts[indices[i + 1]].vertex;
			const glm::vec3& v2 = verts[indices[i + 2]].vertex;

			const glm::vec2& uv0 = verts[indices[i + 0]].uv;
			const glm::vec2& uv1 = verts[indices[i + 1]].uv;
			const glm::vec2& uv2 = verts[indices[i + 2]].uv;

			glm::vec3 faceTangent = CalculateTangent(v0, v1, v2, uv0, uv1, uv2);
			verts[indices[i + 0]].tangent += faceTangent;
			verts[indices[i + 1]].tangent += faceTangent;
			verts[indices[i + 2]].tangent += faceTangent;
		}
		for (auto& v : verts) 
		{
			v.tangent = glm::normalize(v.tangent);
			// Gram-Schmidt 직교화
			v.tangent = glm::normalize(v.tangent - v.normal * glm::dot(v.normal, v.tangent));
		}

		mesh->GetBoundingBox().Set(min, max);

		mesh->SetVertex(std::move(verts));
		mesh->SetIndices(std::move(indices));

		mesh->Build(context);

		return mesh;
	}

	SH_EDITOR_API auto MeshImporter::GetName() const -> const char*
	{
		return name;
	}
	SH_EDITOR_API auto MeshImporter::Serialize() const -> core::Json
	{
		return core::Json{};
	}
	SH_EDITOR_API void MeshImporter::Deserialize(const core::Json& json)
	{
	}
}
