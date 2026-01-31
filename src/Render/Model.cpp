#include "Model.h"
#include "Core/Logger.h"

#include <queue>
namespace sh::render
{
	Model::Model()
	{
		rootNode = std::make_unique<Node>();
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
	SH_RENDER_API void Model::AddMeshes(std::unique_ptr<Node>&& node)
	{
		rootNode = std::move(node);
		std::queue<Node*> q;
		q.push(rootNode.get());
		while (!q.empty())
		{
			Node* curNode = q.front();
			q.pop();
			if (curNode->mesh != nullptr)
			{
				meshes.push_back(curNode->mesh);
			}
			for (auto& child : curNode->children)
				q.push(child.get());
		}
	}
	SH_RENDER_API auto sh::render::Model::GetMeshes() const -> const core::SVector<Mesh*>&
	{
		return meshes;
	}
	SH_RENDER_API auto Model::GetRootNode() const -> const Node*
	{
		return rootNode.get();
	}
	SH_RENDER_API auto Model::Serialize() const -> core::Json
	{
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