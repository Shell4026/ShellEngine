#include "Model.h"
#include "Core/Logger.h"

#include <queue>
namespace sh::render
{
	Model::Model(std::vector<Node> nodes)
	{
		if (!nodes.empty())
			SetName(nodes.front().name);

		this->nodes = std::move(nodes);

		for (Node& node : this->nodes)
		{
			if (node.mesh != nullptr)
				meshes.push_back(node.mesh);
		}
	}
	Model::~Model()
	{
	}
	SH_RENDER_API void Model::Destroy()
	{
		for (auto mesh : meshes)
		{
			if (mesh != nullptr)
				mesh->Destroy();
		}
		Super::Destroy();
		std::vector<int> a;
		a.push_back(0);
	}

	SH_RENDER_API auto Model::Serialize() const -> core::Json
	{
		// 노트 데이터는 직렬화 안 함 (ModelAsset에서 따로 저장)
		core::Json mainJson = Super::Serialize();
		for (auto mesh : meshes)
		{
			if (mesh != nullptr)
				mainJson["Mesh"].push_back(mesh->Serialize());
		}
		return mainJson;
	}
	SH_RENDER_API void Model::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		if (meshes.size() != json["Mesh"].size())
		{
			SH_ERROR_FORMAT("{}: Can't deserialize mesh", GetName().ToString());
			return;
		}
		int idx = 0;
		for (auto& meshJson : json["Mesh"])
		{
			if (meshes[idx] != nullptr)
				meshes[idx]->Deserialize(meshJson);
			++idx;
		}
	}
}//namespace