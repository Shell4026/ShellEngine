#include "Asset/ModelLoader.h"
#include "Asset/ModelAsset.h"

#include "Core/SObject.h"
#include "Core/Logger.h"
#include "Core/FileSystem.h"

#include "Render/Model.h"
#include "Render/SkinnedMesh.h"

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
#include <unordered_map>
#include <map>
#include <queue>
#include <numeric>
namespace sh::game
{
	SH_GAME_API auto ModelLoader::LoadObj(const std::filesystem::path& path) const -> render::Model*
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

		const float minLimit = std::numeric_limits<float>::min();
		const float maxLimit = std::numeric_limits<float>::max();
		glm::vec3 min{ maxLimit, maxLimit, maxLimit }, max{ minLimit, minLimit, minLimit };
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
				glm::vec3 normal{ 0.f, 1.f, 0.f };
				if(index.normal_index != -1)
				{
					normal.x = attrib.normals[3 * index.normal_index + 0];
					normal.y = attrib.normals[3 * index.normal_index + 1];
					normal.z = attrib.normals[3 * index.normal_index + 2];
				}
				glm::vec2 uv{ 0.f, 0.f };
				if (index.texcoord_index != -1)
				{
					uv.x = attrib.texcoords[2 * index.texcoord_index + 0];
					uv.y = 1 - attrib.texcoords[2 * index.texcoord_index + 1];
				}

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

		auto mesh = core::SObject::Create<render::Mesh>();
		mesh->GetBoundingBox().Set(min, max);
		mesh->SetName("Mesh");

		mesh->SetVertex(std::move(verts));
		mesh->SetIndices(std::move(indices));

		mesh->Build(context);

		std::vector<render::Model::Node> nodes;
		render::Model::Node& rootNode = nodes.emplace_back();
		rootNode.name = "root";

		rootNode.childrenIdx.push_back(nodes.size());

		render::Model::Node& node = nodes.emplace_back();
		node.mesh = mesh;

		auto model = core::SObject::Create<render::Model>(std::move(nodes));
		model->SetName(path.filename().u8string());
		return model;
	}
	SH_GAME_API auto ModelLoader::LoadGLTF(const std::filesystem::path& dir) const -> render::Model*
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
		if (gltfModel.scenes.empty())
		{
			SH_ERROR_FORMAT("Scene is empty: {}", dir.u8string());
			return nullptr;
		}

		const auto& scene = gltfModel.scenes[0];

		std::vector<render::Model::Node> nodes;
		nodes.reserve(gltfModel.nodes.size());
		render::Model::Node rootNode{};
		rootNode.name = "root";
		nodes.push_back(std::move(rootNode));

		using GLTFNodeIdx = int;
		using NodeIdx = int;
		std::map<GLTFNodeIdx, NodeIdx> nodeMap;

		std::queue<std::pair<GLTFNodeIdx, NodeIdx>> nodeQ{}; // 노드idx, 부모idx
		for (int nodeIdx : scene.nodes)
			nodeQ.push({ nodeIdx, 0 });

		while (!nodeQ.empty())
		{
			auto [gltfNodeIdx, parentNodeIdx] = nodeQ.front();
			nodeQ.pop();

			const int idx = nodes.size();
			nodeMap.insert({ gltfNodeIdx, idx });

			nodes[parentNodeIdx].childrenIdx.push_back(idx);

			tinygltf::Node& gltfNode = gltfModel.nodes[gltfNodeIdx];

			for (int childIdx : gltfNode.children)
				nodeQ.push({ childIdx, idx });

			render::Model::Node& node = nodes.emplace_back();
			node.name = gltfNode.name;
			node.skeletonIdx = gltfNode.skin; // -1이면 스킨 없음

			glm::mat4 matrix{ 1.0f };
			if (gltfNode.matrix.size() == 16)
			{
				matrix = glm::make_mat4x4(gltfNode.matrix.data());
			}
			else
			{
				if (gltfNode.translation.size() == 3)
					matrix = glm::translate(matrix, glm::vec3{ glm::make_vec3(gltfNode.translation.data()) });
				if (gltfNode.rotation.size() == 4)
				{
					glm::quat q{ glm::make_quat(gltfNode.rotation.data()) };
					matrix *= glm::mat4(q);
				}
				if (gltfNode.scale.size() == 3)
				{
					matrix = glm::scale(matrix, glm::vec3{ glm::make_vec3(gltfNode.scale.data()) });
				}
			}
			node.modelMatrix = matrix;

			if (gltfNode.mesh >= 0)
			{
				const float minLimit = std::numeric_limits<float>::min();
				const float maxLimit = std::numeric_limits<float>::max();
				glm::vec3 min{ maxLimit, maxLimit, maxLimit }, max{ minLimit, minLimit, minLimit };

				std::vector<render::Mesh::Vertex> verts;
				std::vector<uint32_t> indices;
				std::vector<render::SkinnedMesh::BoneVertex> boneVerts;
				std::vector<render::SubMesh> subMeshes;
				bool bSkinned = (gltfNode.skin >= 0);

				const tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfNode.mesh];

				for (auto& primitive : gltfMesh.primitives)
				{
					render::SubMesh& subMesh = subMeshes.emplace_back();

					uint32_t firstIndex = static_cast<uint32_t>(indices.size());
					uint32_t vertexStart = static_cast<uint32_t>(verts.size());

					const float* positionBuffer = nullptr;
					const float* normalsBuffer = nullptr;
					const float* texCoordsBuffer = nullptr;
					const uint8_t* jointsBuffer = nullptr;
					const float* weightsBuffer = nullptr;
					int jointsCompType = 0;
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
					if (bSkinned)
					{
						if (auto it = primitive.attributes.find("JOINTS_0"); it != primitive.attributes.end())
						{
							const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
							const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
							jointsBuffer = &gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset];
							jointsCompType = accessor.componentType;
						}
						if (auto it = primitive.attributes.find("WEIGHTS_0"); it != primitive.attributes.end())
						{
							const tinygltf::Accessor& accessor = gltfModel.accessors[it->second];
							const tinygltf::BufferView& view = gltfModel.bufferViews[accessor.bufferView];
							weightsBuffer = reinterpret_cast<const float*>(&gltfModel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);
						}
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

						if (bSkinned && jointsBuffer != nullptr && weightsBuffer != nullptr)
						{
							render::SkinnedMesh::BoneVertex bv{};
							if (jointsCompType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
							{
								const uint8_t* j = jointsBuffer + v * 4;
								bv.boneIndices = glm::ivec4{ j[0], j[1], j[2], j[3] };
							}
							else // UNSIGNED_SHORT
							{
								const uint16_t* j = reinterpret_cast<const uint16_t*>(jointsBuffer) + v * 4;
								bv.boneIndices = glm::ivec4{ j[0], j[1], j[2], j[3] };
							}
							bv.boneWeights = glm::make_vec4(&weightsBuffer[v * 4]);
							boneVerts.push_back(bv);
						}
					}
					// indices
					{
						const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
						const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
						const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

						subMesh.indexOffset = indices.size();
						subMesh.indexCount = static_cast<uint32_t>(accessor.count);

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
				} // for (auto& primitive : gltfMesh.primitives)
				CreateTangents(verts, indices);

				render::Mesh* meshPtr = nullptr;
				if (bSkinned)
				{
					auto skinnedMesh = core::SObject::Create<render::SkinnedMesh>();
					skinnedMesh->SetName(gltfNode.name);
					skinnedMesh->GetBoundingBox().Set(min, max);
					skinnedMesh->SetVertex(std::move(verts));
					skinnedMesh->SetIndices(std::move(indices));
					skinnedMesh->SetBoneVertices(std::move(boneVerts));
					skinnedMesh->SetSubMeshes(std::move(subMeshes));

					const tinygltf::Skin& gltfSkin = gltfModel.skins[gltfNode.skin];
					if (gltfSkin.inverseBindMatrices >= 0)
					{
						const tinygltf::Accessor& acc = gltfModel.accessors[gltfSkin.inverseBindMatrices];
						const tinygltf::BufferView& bv = gltfModel.bufferViews[acc.bufferView];
						const float* ibmData = reinterpret_cast<const float*>(
							&gltfModel.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]);
						std::vector<glm::mat4> ibms(acc.count);
						for (std::size_t i = 0; i < acc.count; ++i)
							ibms[i] = glm::make_mat4x4(ibmData + i * 16);
						skinnedMesh->SetInverseBindMatrices(std::move(ibms));
					}

					skinnedMesh->Build(context);
					meshPtr = skinnedMesh;
				}
				else
				{
					auto mesh = core::SObject::Create<render::Mesh>();
					mesh->SetName(gltfNode.name);
					mesh->GetBoundingBox().Set(min, max);
					mesh->SetVertex(std::move(verts));
					mesh->SetIndices(std::move(indices));
					mesh->SetSubMeshes(std::move(subMeshes));
					mesh->Build(context);
					meshPtr = mesh;
				}
				node.mesh = meshPtr;
			}
		} // while (!nodeQ.empty())

		// 스킨 데이터
		std::vector<render::Skeleton> skeletons;
		skeletons.reserve(gltfModel.skins.size());
		for (const tinygltf::Skin& gltfSkin : gltfModel.skins)
		{
			render::Skeleton& skeleton = skeletons.emplace_back();

			const std::size_t jointCount = gltfSkin.joints.size();
			skeleton.joints.resize(jointCount);

			for (std::size_t i = 0; i < jointCount; ++i)
			{
				const GLTFNodeIdx jNodeIdx = gltfSkin.joints[i];
				if (auto it = nodeMap.find(jNodeIdx); it != nodeMap.end())
					skeleton.joints[i].nodeIdx = it->second;
			}

			if (gltfSkin.inverseBindMatrices >= 0)
			{
				const tinygltf::Accessor& acc = gltfModel.accessors[gltfSkin.inverseBindMatrices];
				const tinygltf::BufferView& bv = gltfModel.bufferViews[acc.bufferView];
				const float* ibmData = reinterpret_cast<const float*>(
					&gltfModel.buffers[bv.buffer].data[acc.byteOffset + bv.byteOffset]);
				for (std::size_t i = 0; i < jointCount && i < acc.count; ++i)
				{
					skeleton.joints[i].inverseBindMat = glm::make_mat4x4(ibmData + i * 16);
				}
			}
		}

		auto model = core::SObject::Create<render::Model>(std::move(nodes));
		model->SetName(dir.filename().u8string());
		model->SetSkeletons(std::move(skeletons));
		return model;
	}
	auto ModelLoader::CalculateTangent(
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
		const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) -> glm::vec3
	{
		glm::vec3 e0 = v1 - v0;
		glm::vec3 e1 = v2 - v0;
		glm::vec2 deltaUV0 = uv1 - uv0;
		glm::vec2 deltaUV1 = uv2 - uv0;

		const float delta = std::max(deltaUV0.x * deltaUV1.y - deltaUV1.x * deltaUV0.y, std::numeric_limits<float>::epsilon());

		float f = 1.0f / delta;

		float x = f * (deltaUV1.y * e0.x - deltaUV0.y * e1.x);
		float y = f * (deltaUV1.y * e0.y - deltaUV0.y * e1.y);
		float z = f * (deltaUV1.y * e0.z - deltaUV0.y * e1.z);
		return glm::vec3{ x, y, z };
	}
	void ModelLoader::CreateTangents(std::vector<render::Mesh::Vertex>& verts, const std::vector<uint32_t>& indices)
	{
		for (std::size_t i = 0; i < indices.size(); i += 3)
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

	SH_GAME_API auto ModelLoader::Load(const std::filesystem::path& path) const -> core::SObject*
	{
		std::string ext = path.extension().string();
		if (ext == ".obj")
			return LoadObj(path);
		else if (ext == ".glb")
			return LoadGLTF(path);
		return nullptr;
	}

	SH_GAME_API auto ModelLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a model!", asset.GetAssetUUID().ToString());
			return nullptr;
		}
		const auto& modelAsset = static_cast<const game::ModelAsset&>(asset);
		if (modelAsset.GetData().empty())
			return nullptr;

		auto model = core::SObject::Create<render::Model>(modelAsset.GetData());
		model->SetUUID(asset.GetAssetUUID());
		model->SetSkeletons(modelAsset.GetSkeletonData());
		return model;
	}

	SH_GAME_API auto ModelLoader::GetAssetName() const -> const char*
	{
		return ASSET_NAME;
	}
}//namespace