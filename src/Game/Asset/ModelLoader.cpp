#include "Asset/ModelLoader.h"
#include "Asset/ModelAsset.h"

#include "Core/SObject.h"
#include "Core/Logger.h"
#include "Core/FileSystem.h"

#include "Render/Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "External/tinyobjloader/tiny_obj_loader.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../external/tinygltf/tiny_gltf.h"

#include "glm/gtc/type_ptr.hpp"
#include <fmt/core.h>

#include <string>
#include <cstdint>
#include <limits>
namespace sh::game
{
	SH_GAME_API auto ModelLoader::LoadObj(const std::filesystem::path& path) -> render::Model*
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
		{
			SH_ERROR_FORMAT("Can't load {}!", path.u8string());
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

		CreateTangents(verts, indices);

		auto model = core::SObject::Create<render::Model>();
		model->SetName(path.filename().u8string());

		auto mesh = core::SObject::Create<render::Mesh>();
		mesh->GetBoundingBox().Set(min, max);
		mesh->SetName("Mesh");

		mesh->SetVertex(std::move(verts));
		mesh->SetIndices(std::move(indices));

		mesh->Build(context);

		auto rootNode = std::make_unique<render::Model::Node>();
		auto node = std::make_unique<render::Model::Node>();
		node->mesh = mesh;

		rootNode->children.push_back(std::move(node));

		model->AddMeshes(std::move(rootNode));
		return model;
	}
	SH_GAME_API auto ModelLoader::LoadGLTF(const std::filesystem::path& dir) -> render::Model*
	{
		static tinygltf::TinyGLTF gltfContext;
		tinygltf::Model gltfModel;
		std::string error, warning;

		auto binary = core::FileSystem::LoadBinary(dir);
		if (!binary.has_value())
			return nullptr;
		
		if (!gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, binary.value().data(), binary.value().size()))
		{
			SH_ERROR_FORMAT("{}", error);
			return nullptr;
		}
		const auto& scene = gltfModel.scenes[0];

		auto model = core::SObject::Create<render::Model>();
		model->SetName(dir.filename().u8string());

		auto rootNode = std::make_unique<render::Model::Node>();

		std::queue<std::pair<int, render::Model::Node*>> nodeQ{};
		for (int nodeIdx : scene.nodes)
			nodeQ.push({ nodeIdx, rootNode.get() });
		
		while (!nodeQ.empty())
		{
			auto [nodeIdx, parentNode] = nodeQ.front();
			nodeQ.pop();

			auto modelNode = std::make_unique<render::Model::Node>();

			tinygltf::Node& node = gltfModel.nodes[nodeIdx];
			modelNode->name = node.name;

			for (int childIdx : node.children)
				nodeQ.push({ childIdx, modelNode.get() });

			glm::mat4 matrix{ 1.0f };
			if (node.matrix.size() == 16)
			{
				matrix = glm::make_mat4x4(node.matrix.data());
			}
			else
			{
				if (node.translation.size() == 3)
					matrix = glm::translate(matrix, glm::vec3{ glm::make_vec3(node.translation.data()) });
				if (node.rotation.size() == 4)
				{
					glm::quat q{ glm::make_quat(node.rotation.data()) };
					matrix *= glm::mat4(q);
				}
				if (node.scale.size() == 3)
				{
					matrix = glm::scale(matrix, glm::vec3{ glm::make_vec3(node.scale.data()) });
				}
			}
			modelNode->modelMatrix = matrix;

			if (node.mesh >= 0)
			{
				auto mesh = core::SObject::Create<render::Mesh>();
				mesh->SetName(node.name);
				modelNode->mesh = mesh;

				const float minLimit = std::numeric_limits<float>::min();
				const float maxLimit = std::numeric_limits<float>::max();
				glm::vec3 min{ maxLimit, maxLimit, maxLimit }, max{ minLimit, minLimit, minLimit };

				std::vector<render::Mesh::Vertex> verts;
				std::vector<uint32_t> indices;

				const tinygltf::Mesh& gltfMesh = gltfModel.meshes[node.mesh];

				uint32_t indexCount = 0;
				for (auto& primitive : gltfMesh.primitives) // 면의 단위
				{
					uint32_t firstIndex = static_cast<uint32_t>(indices.size());
					uint32_t vertexStart = static_cast<uint32_t>(verts.size());

					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					std::size_t vertexCount = 0;

					if (auto it = primitive.attributes.find("POSITION"); it != primitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
						const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						positionBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						vertexCount = accessor.count;
					}
					if (auto it = primitive.attributes.find("NORMAL"); it != primitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
						const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						normalsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					if (auto it = primitive.attributes.find("TEXCOORD_0"); it != primitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
						const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
						texCoordsBuffer = reinterpret_cast<const float*>(&(gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					}
					for (size_t v = 0; v < vertexCount; v++)
					{
						render::Mesh::Vertex vert{};
						vert.vertex = glm::make_vec3(&positionBuffer[v * 3]);
						vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
						vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
						verts.push_back(vert);

						min.x = std::min(min.x, vert.vertex.x);
						min.y = std::min(min.y, vert.vertex.y);
						min.z = std::min(min.z, vert.vertex.z);

						max.x = std::max(max.x, vert.vertex.x);
						max.y = std::max(max.y, vert.vertex.y);
						max.z = std::max(max.z, vert.vertex.z);
					}
					// indicies
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
						const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

						indexCount += static_cast<uint32_t>(accessor.count);

						switch (accessor.componentType)
						{
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						{
							const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
								indices.push_back(buf[index] + vertexStart);
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
								indices.push_back(buf[index] + vertexStart);
							break;
						}
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
							for (size_t index = 0; index < accessor.count; index++)
								indices.push_back(buf[index] + vertexStart);
							break;
						}
						default:
							std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
							return nullptr;
						}
					}
				}
				CreateTangents(verts, indices);

				mesh->GetBoundingBox().Set(min, max);

				mesh->SetVertex(std::move(verts));
				mesh->SetIndices(std::move(indices));

				mesh->Build(context);
			}
			parentNode->children.push_back(std::move(modelNode));
		}
		model->AddMeshes(std::move(rootNode));
		return model;
	}
	auto ModelLoader::CalculateTangent(
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
	void ModelLoader::CreateTangents(std::vector<render::Mesh::Vertex>& verts, const std::vector<uint32_t>& indices)
	{
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
	}
	ModelLoader::ModelLoader(const render::IRenderContext& context) :
		context(context)
	{
	}

	SH_GAME_API auto ModelLoader::Load(const std::filesystem::path& path) -> core::SObject*
	{
		std::string ext = path.extension().string();
		if (ext == ".obj")
			return LoadObj(path);
		else if (ext == ".glb")
			return LoadGLTF(path);
		return nullptr;
	}

	SH_GAME_API auto ModelLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a model!", asset.GetAssetUUID().ToString());
			return nullptr;
		}
		const auto& modelAsset = static_cast<const game::ModelAsset&>(asset);
		const game::ModelAsset::ModelHeader header = modelAsset.GetHeader();
		const game::ModelAsset::ModelData modelData = modelAsset.GetData();
		const uint8_t* cursor = modelData.dataPtr;

		std::vector<render::Mesh*> meshes;
		meshes.reserve(header.meshCount);

		// 메쉬 로드
		for (size_t i = 0; i < header.meshCount; ++i)
		{
			game::ModelAsset::MeshHeader meshHeader;
			if (cursor + sizeof(meshHeader) > modelData.dataPtr + modelData.size)
				return nullptr;

			std::memcpy(&meshHeader, cursor, sizeof(game::ModelAsset::MeshHeader));
			cursor += sizeof(meshHeader);

			// Vertex 배열 읽기
			std::vector<render::Mesh::Vertex> vertices(meshHeader.vertexCount);
			if (meshHeader.vertexCount > 0)
			{
				size_t vertexBytes = meshHeader.vertexCount * sizeof(render::Mesh::Vertex);
				if (cursor + vertexBytes > modelData.dataPtr + modelData.size)
					return nullptr;
				std::memcpy(vertices.data(), cursor, vertexBytes);
				cursor += vertexBytes;
			}
			// Index 배열 읽기
			std::vector<uint32_t> indices(meshHeader.indexCount);
			if (meshHeader.indexCount > 0)
			{
				size_t indexBytes = meshHeader.indexCount * sizeof(uint32_t);
				if (cursor + indexBytes > modelData.dataPtr + modelData.size)
					return nullptr;
				std::memcpy(indices.data(), cursor, indexBytes);
				cursor += indexBytes;
			}
			// Mesh 객체 생성
			auto mesh = core::SObject::Create<render::Mesh>();
			mesh->SetUUID(meshHeader.uuid);
			mesh->SetVertex(std::move(vertices));
			mesh->SetIndices(std::move(indices));
			mesh->Build(context);

			meshes.push_back(mesh);
		}
		// 노드 로드
		std::vector<game::ModelAsset::NodeHeader> nodeHeaders(header.nodeCount);
		if (header.nodeCount > 0)
		{
			size_t nodeBytes = header.nodeCount * sizeof(game::ModelAsset::NodeHeader);
			if (cursor + nodeBytes > modelData.dataPtr + modelData.size)
				return nullptr;
			std::memcpy(nodeHeaders.data(), cursor, nodeBytes);
			cursor += nodeBytes;
		}
		std::vector<std::unique_ptr<render::Model::Node>> nodes(header.nodeCount);
		std::vector<render::Model::Node*> rawNodes(header.nodeCount);
		for (size_t i = 0; i < nodeHeaders.size(); ++i)
		{
			nodes[i] = std::make_unique<render::Model::Node>();
			nodes[i]->modelMatrix = nodeHeaders[i].modelMatrix;
			nodes[i]->name = nodeHeaders[i].name;
			rawNodes[i] = nodes[i].get();

			if (nodeHeaders[i].meshIndex >= 0 && static_cast<size_t>(nodeHeaders[i].meshIndex) < meshes.size())
				nodes[i]->mesh = meshes[nodeHeaders[i].meshIndex];
		}
		render::Model::Node* rootNodePtr = nullptr;
		for (size_t i = 0; i < nodeHeaders.size(); ++i)
		{
			int32_t parentIdx = nodeHeaders[i].parentIndex;
			if (parentIdx >= 0)
				rawNodes[parentIdx]->children.push_back(std::move(nodes[i]));
			else
				rootNodePtr = nodes[i].release();
		}

		auto model = core::SObject::Create<render::Model>();
		model->AddMeshes(std::unique_ptr<render::Model::Node>(rootNodePtr));

		return model;
	}

	SH_GAME_API auto ModelLoader::GetAssetName() const -> const char*
	{
		return ASSET_NAME;
	}
}//namespace