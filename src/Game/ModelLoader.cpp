#include "ModelLoader.h"

#include "Render/Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "External/tinyobjloader/tiny_obj_loader.h"

#include <string>
#include <fmt/core.h>

namespace sh::game
{
	ModelLoader::ModelLoader(const render::Renderer& renderer) :
		renderer(renderer)
	{
	}

	auto ModelLoader::Load(std::string_view filename) -> std::unique_ptr<render::Mesh>
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		auto mesh = std::make_unique<render::Mesh>();
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.data()))
		{
			fmt::print("Can't load {}!\n", filename);
		}

		std::unordered_map<glm::vec3, uint32_t> uniqueVerts;
		std::vector<glm::vec3> verts;
		std::vector<glm::vec2> uvs;
		std::vector<uint32_t> indices;
		for (const auto& shape : shapes)
		{
			uint32_t n = 0;
			for (const auto& index : shape.mesh.indices) 
			{
				glm::vec3 vert{
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				glm::vec2 uv
				{
					attrib.texcoords[2 * index.texcoord_index + 0],
					1 - attrib.texcoords[2 * index.texcoord_index + 1],
				};

				auto it = uniqueVerts.find(vert);
				if (it == uniqueVerts.end())
				{
					uniqueVerts.insert({ vert, n });

					verts.push_back(vert);
					uvs.push_back(uv);
					indices.push_back(n++);
				}
				else
					indices.push_back(it->second);
			}
		}

		mesh->SetVertex(std::move(verts));
		mesh->SetAttribute(render::ShaderAttribute<glm::vec2>{"uvs", std::move(uvs)});
		mesh->SetIndices(std::move(indices));

		mesh->Build(renderer);

		return mesh;
	}

}
