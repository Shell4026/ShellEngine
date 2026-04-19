#include "Asset/ModelAsset.h"

namespace sh::game
{
    ModelAsset::ModelAsset() :
        Asset(ASSET_NAME)
    {
    }
    ModelAsset::ModelAsset(const render::Model& model) :
        Asset(ASSET_NAME),
        modelPtr(&model)
    {
        assetUUID = modelPtr->GetUUID();
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
    }
    SH_GAME_API auto ModelAsset::GetData() const -> const std::vector<render::Model::Node>&
    {
        if (modelPtr.IsValid())
            return modelPtr->GetNodes();
        return nodeData;
    }
    SH_GAME_API auto ModelAsset::GetSkeletonData() const -> const std::vector<render::Skeleton>&
    {
        if (modelPtr.IsValid())
            return modelPtr->GetSkeletons();
        return skeletonData;
    }

    SH_GAME_API void ModelAsset::SetAssetData() const
    {
        if (!modelPtr.IsValid())
            return;

        const std::vector<render::Model::Node>& nodes = modelPtr->GetNodes();
        const std::vector<render::Skeleton>& skeletons = modelPtr->GetSkeletons();

        ModelHeader modelHeader{};
        modelHeader.nodeCount = nodes.size();
        modelHeader.skeletonCount = skeletons.size();

        std::size_t cursor = 0;
        std::size_t totalSize = sizeof(ModelHeader);
        data.resize(totalSize);
        std::memcpy(data.data() + cursor, &modelHeader, sizeof(ModelHeader));
        cursor = totalSize;

        for (std::size_t i = 0; i < modelHeader.nodeCount; ++i)
        {
            NodeHeader nodeHeader{};
            nodeHeader.nameSize = nodes[i].name.size();
            nodeHeader.modelMatrix = nodes[i].modelMatrix;
            nodeHeader.skeletonIdx = nodes[i].skeletonIdx;
            nodeHeader.childrenCount = nodes[i].childrenIdx.size();
            if (nodes[i].mesh != nullptr)
                nodeHeader.meshUUID = nodes[i].mesh->GetUUID();

            totalSize += sizeof(NodeHeader);
            data.resize(totalSize);
            std::memcpy(data.data() + cursor, &nodeHeader, sizeof(NodeHeader));
            cursor = totalSize;

            totalSize += sizeof(char) * nodeHeader.nameSize;
            data.resize(totalSize);
            std::memcpy(data.data() + cursor, nodes[i].name.data(), sizeof(char) * nodeHeader.nameSize);
            cursor = totalSize;

            totalSize += sizeof(int) * nodeHeader.childrenCount;
            data.resize(totalSize);
            std::memcpy(data.data() + cursor, nodes[i].childrenIdx.data(), sizeof(int) * nodeHeader.childrenCount);
            cursor = totalSize;
        }

        for (std::size_t i = 0; i < modelHeader.skeletonCount; ++i)
        {
            SkeletonHeader skeletonHeader{};
            skeletonHeader.jointCount = skeletons[i].joints.size();

            totalSize += sizeof(SkeletonHeader);
            data.resize(totalSize);
            std::memcpy(data.data() + cursor, &skeletonHeader, sizeof(SkeletonHeader));
            cursor = totalSize;

            for (std::size_t j = 0; j < skeletonHeader.jointCount; ++j)
            {
                const render::Skeleton::Joint& joint = skeletons[i].joints[j];

                totalSize += sizeof(int);
                data.resize(totalSize);
                std::memcpy(data.data() + cursor, &joint.nodeIdx, sizeof(int));
                cursor = totalSize;

                totalSize += sizeof(glm::mat4);
                data.resize(totalSize);
                std::memcpy(data.data() + cursor, &joint.inverseBindMat, sizeof(glm::mat4));
                cursor = totalSize;
            }
        }
    }
    SH_GAME_API auto ModelAsset::ParseAssetData() -> bool
    {
        modelPtr.Reset();

        if (data.size() < sizeof(ModelHeader))
            return false;

        ModelHeader modelHeader{};

        std::size_t cursor = 0;
        std::memcpy(&modelHeader, data.data(), sizeof(ModelHeader));
        cursor += sizeof(ModelHeader);

        nodeData.resize(modelHeader.nodeCount);

        for (std::size_t i = 0; i < nodeData.size(); ++i)
        {
            NodeHeader nodeHeader{};
            if (cursor + sizeof(NodeHeader) > data.size())
                return false;
            std::memcpy(&nodeHeader, data.data() + cursor, sizeof(NodeHeader));
            cursor += sizeof(NodeHeader);
            const core::UUID meshUUID{ nodeHeader.meshUUID };
            nodeData[i].skeletonIdx = nodeHeader.skeletonIdx;
            nodeData[i].modelMatrix = nodeHeader.modelMatrix;
            nodeData[i].mesh = meshUUID.IsEmpty() ? nullptr : static_cast<render::Mesh*>(core::SObject::GetSObjectUsingResolver(meshUUID));

            if (cursor + sizeof(char) * nodeHeader.nameSize > data.size())
                return false;
            nodeData[i].name.resize(nodeHeader.nameSize);
            std::memcpy(nodeData[i].name.data(), data.data() + cursor, sizeof(char) * nodeHeader.nameSize);
            cursor += sizeof(char) * nodeHeader.nameSize;

            if (cursor + sizeof(int) * nodeHeader.childrenCount > data.size())
                return false;
            nodeData[i].childrenIdx.resize(nodeHeader.childrenCount);
            std::memcpy(nodeData[i].childrenIdx.data(), data.data() + cursor, sizeof(int) * nodeHeader.childrenCount);
            cursor += sizeof(int) * nodeHeader.childrenCount;
        }

        skeletonData.resize(modelHeader.skeletonCount);

        for (std::size_t i = 0; i < skeletonData.size(); ++i)
        {
            SkeletonHeader skeletonHeader{};
            if (cursor + sizeof(SkeletonHeader) > data.size())
                return false;
            std::memcpy(&skeletonHeader, data.data() + cursor, sizeof(SkeletonHeader));
            cursor += sizeof(SkeletonHeader);

            skeletonData[i].joints.resize(skeletonHeader.jointCount);

            for (std::size_t j = 0; j < skeletonHeader.jointCount; ++j)
            {
                if (cursor + sizeof(int) + sizeof(glm::mat4) > data.size())
                    return false;

                std::memcpy(&skeletonData[i].joints[j].nodeIdx, data.data() + cursor, sizeof(int));
                cursor += sizeof(int);

                std::memcpy(&skeletonData[i].joints[j].inverseBindMat, data.data() + cursor, sizeof(glm::mat4));
                cursor += sizeof(glm::mat4);
            }
        }

        return true;
    }
}//namespace