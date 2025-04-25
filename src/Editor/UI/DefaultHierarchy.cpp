#include "UI/DefaultHierarchy.h"
#include "EditorWorld.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"

#include "Core/Logger.h"

#include <queue>

namespace sh::editor
{
	SH_EDITOR_API void ModelHierarchy::OnHierarchyDraged(EditorWorld& world, const ImGuiPayload& payload)
	{
		render::Model* model = *reinterpret_cast<render::Model**>(payload.Data);

		auto obj = world.AddGameObject(model->GetName().ToString());

		auto rootNode = model->GetRootNode();
		std::queue<std::pair<const render::Model::Node*, game::GameObject*>> nodeQ;
		for (auto& child : rootNode->children)
			nodeQ.push({ child.get(), obj });

		while (!nodeQ.empty())
		{
			auto [node, parentObj] = nodeQ.front();
			nodeQ.pop();

			auto meshObj = world.AddGameObject(node->name);
			meshObj->transform->SetParent(parentObj->transform);
			meshObj->transform->SetModelMatrix(node->modelMatrix);

			if (node->mesh != nullptr)
			{
				auto meshRenderer = meshObj->AddComponent<game::MeshRenderer>();
				meshRenderer->SetMesh(node->mesh);
			}
			for (auto& child : node->children)
				nodeQ.push({ child.get(), meshObj });
		}
	}
}//namespace