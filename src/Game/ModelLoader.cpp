#include "PCH.h"
#include "ModelLoader.h"

#include "Core/SObject.h"
#include "Core/Logger.h"

#include "Render/Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "External/tinyobjloader/tiny_obj_loader.h"

#include <string>
#include <fmt/core.h>

namespace sh::game
{
	ModelLoader::ModelLoader(const render::IRenderContext& context) :
		context(context)
	{
	}

	auto ModelLoader::Load(std::string_view filename) -> render::Mesh*
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
		std::vector<glm::vec3> verts;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;
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

					verts.push_back(vert);
					uvs.push_back(uv);
					normals.push_back(normal);
					indices.push_back(n++);
				}
				else
					indices.push_back(it->second);
			}
		}
		mesh->GetBoundingBox().Set(min, max);

		mesh->SetVertex(std::move(verts));
		mesh->SetUV(uvs);
		mesh->SetNormal(normals);
		mesh->SetIndices(std::move(indices));

		mesh->Build(context);

		return mesh;
	}

}
