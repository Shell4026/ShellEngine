#include "ModelAsset.h"

#include <map>
namespace sh::game
{
    void Flatten(
        const render::Model::Node* node, 
        int32_t parentIdx,
        int32_t& currentIdx, 
        std::vector<ModelAsset::NodeHeader>& flatNodes,
        const std::map<const render::Mesh*, uint32_t>& meshMap)
    {
        const int32_t idx = currentIdx++;
        flatNodes.emplace_back();
        ModelAsset::NodeHeader& header = flatNodes.back();

        header.parentIndex = parentIdx;
        header.modelMatrix = node->modelMatrix;

        if (core::IsValid(node->mesh))
        {
            auto it = meshMap.find(node->mesh);
            header.meshIndex = (it != meshMap.end()) ? static_cast<int32_t>(it->second) : -1;
        }
        else
        {
            header.meshIndex = -1;
        }

        strncpy(header.name, node->name.c_str(), sizeof(header.name) - 1);
        header.name[sizeof(header.name) - 1] = '\0';

        for (const auto& child : node->children)
            Flatten(child.get(), idx, currentIdx, flatNodes, meshMap);
    }

    auto ModelAsset::SetHeader(ModelHeader& header) const -> std::vector<NodeHeader>
    {
        if (!modelPtr.IsValid())
            return std::vector<NodeHeader>{};

        const auto& sourceModel = *modelPtr;
        const auto& meshes = sourceModel.GetMeshes();
        const auto* rootNode = sourceModel.GetRootNode();

        if (!rootNode)
            return std::vector<NodeHeader>{};

        std::map<const render::Mesh*, uint32_t> meshToIdxMap;
        for (uint32_t i = 0; i < meshes.size(); ++i)
            meshToIdxMap[meshes[i]] = i;

        std::vector<NodeHeader> nodeHeaders;
        int32_t nodeIdx = 0;
        Flatten(rootNode, -1, nodeIdx, nodeHeaders, meshToIdxMap);

        header.meshCount = meshes.size();
        header.nodeCount = nodeHeaders.size();

        return nodeHeaders;
    }

    SH_GAME_API void ModelAsset::SetAssetData() const
	{
        if (!modelPtr.IsValid())
            return;

        ModelHeader header{};
        std::vector<NodeHeader> nodeHeaders = SetHeader(header);

        const auto& meshes = modelPtr->GetMeshes();
        
        // 사이즈 계산
        size_t totalSize = sizeof(ModelHeader);
        totalSize += nodeHeaders.size() * sizeof(NodeHeader);
        totalSize += meshes.size() * sizeof(MeshHeader);

        std::vector<size_t> meshVertexSizes;
        std::vector<size_t> meshIndexSizes;
        meshVertexSizes.reserve(meshes.size());
        meshIndexSizes.reserve(meshes.size());

        for (const auto* mesh : meshes)
        {
            size_t vertexBytes = mesh->GetVertex().size() * sizeof(render::Mesh::Vertex);
            size_t indexBytes = mesh->GetIndices().size() * sizeof(uint32_t);
            totalSize += vertexBytes;
            totalSize += indexBytes;
            meshVertexSizes.push_back(vertexBytes);
            meshIndexSizes.push_back(indexBytes);
        }
        
        // 바이너리 쓰기
        data.resize(totalSize);
        uint8_t* cursor = data.data();

        std::memcpy(cursor, &header, sizeof(ModelHeader));
        cursor += sizeof(ModelHeader);

        for (size_t i = 0; i < meshes.size(); ++i)
        {
            const render::Mesh* mesh = meshes[i];
            MeshHeader meshHeader;
            meshHeader.uuid = mesh->GetUUID().GetRawData();
            meshHeader.vertexCount = mesh->GetVertex().size();
            meshHeader.indexCount = mesh->GetIndices().size();

            std::memcpy(cursor, &meshHeader, sizeof(MeshHeader));
            cursor += sizeof(MeshHeader);

            if (meshHeader.vertexCount > 0)
            {
                std::memcpy(cursor, mesh->GetVertex().data(), meshVertexSizes[i]);
                cursor += meshVertexSizes[i];
            }

            if (meshHeader.indexCount > 0)
            {
                std::memcpy(cursor, mesh->GetIndices().data(), meshIndexSizes[i]);
                cursor += meshIndexSizes[i];
            }
        }
        if (!nodeHeaders.empty())
        {
            std::memcpy(cursor, nodeHeaders.data(), nodeHeaders.size() * sizeof(NodeHeader));
        }
	}
    SH_GAME_API auto ModelAsset::ParseAssetData() -> bool
    {
        modelPtr.Reset();

        if (data.size() < sizeof(ModelHeader))
            return false;

        std::memcpy(&header, data.data(), sizeof(ModelHeader));
        return true;
    }
    ModelAsset::ModelAsset() :
        Asset(ASSET_NAME)
    {
    }
    ModelAsset::ModelAsset(const render::Model& model) :
        Asset(ASSET_NAME),
        modelPtr(&model)
    {
        assetUUID = modelPtr->GetUUID();
        SetHeader(header);
    }
    ModelAsset::~ModelAsset()
    {
    }
    SH_GAME_API void ModelAsset::SetAsset(const core::SObject& obj)
    {
        if (obj.GetType() != render::Model::GetStaticType())
            return;

        modelPtr = static_cast<const render::Model*>(&obj);
        assetUUID = modelPtr->GetUUID();

        SetHeader(header);
    }
    SH_GAME_API auto ModelAsset::GetHeader() const -> ModelHeader
    {
        return header;
    }
    SH_GAME_API auto ModelAsset::GetData() const -> ModelData
    {
        ModelData modelData;
        modelData.dataPtr = data.data() + sizeof(ModelHeader);
        modelData.size = data.size() - sizeof(ModelHeader);
        return modelData;
    }
}//namespace