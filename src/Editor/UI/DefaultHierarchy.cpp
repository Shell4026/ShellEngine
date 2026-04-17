#include "UI/DefaultHierarchy.h"
#include "EditorWorld.h"

#include "Game/GameObject.h"
#include "Game/Component/Render/MeshRenderer.h"
#include "Game/Component/Render/SkinnedMeshRenderer.h"

#include "Render/SkinnedMesh.h"

#include "Core/Logger.h"

#include <queue>
#include <vector>

namespace sh::editor
{
	SH_EDITOR_API void ModelHierarchy::OnHierarchyDraged(EditorWorld& world, core::SObject& payload)
	{
		render::Model& model = static_cast<render::Model&>(payload);

		auto obj = world.AddGameObject(model.GetName().ToString());

		auto rootNode = model.GetRootNode();
		std::queue<std::pair<const render::Model::Node*, game::GameObject*>> nodeQ;
		for (auto& child : rootNode->children)
			nodeQ.push({ child.get(), obj });

		while (!nodeQ.empty())
		{
			auto [nodeIdx, parentObj] = nodeQ.front();
			nodeQ.pop();

			const render::Model::Node& node = nodes[nodeIdx];

			auto meshObj = world.AddGameObject(node.name);
			meshObj->transform->SetParent(parentObj->transform);
			meshObj->transform->SetModelMatrix(node.modelMatrix);
			nodeMap[nodeIdx] = meshObj;

			if (node.mesh != nullptr)
			{
				if (node.skeletonIdx >= 0 && node.skeletonIdx < static_cast<int>(skeletons.size()))
				{
					auto renderer = meshObj->AddComponent<game::SkinnedMeshRenderer>();
					renderer->SetSkinnedMesh(static_cast<render::SkinnedMesh*>(node.mesh));
					skinnedSetups.push_back({ renderer, node.skeletonIdx });
				}
				else
				{
					auto renderer = meshObj->AddComponent<game::MeshRenderer>();
					renderer->SetMesh(node.mesh);
				}
			}

			for (int childIdx : node.childrenIdx)
				nodeQ.push({ childIdx, meshObj });
		}

		// 모든 GameObject 생성 후 bone Transform 연결
		for (auto& setup : skinnedSetups)
		{
			const render::Skeleton& skeleton = skeletons[setup.skeletonIdx];

			std::vector<game::Transform*> bones;
			bones.reserve(skeleton.joints.size());
			for (const auto& joint : skeleton.joints)
			{
				game::Transform* bone = nullptr;
				if (joint.nodeIdx >= 0 && joint.nodeIdx < static_cast<int>(nodeMap.size()))
					bone = nodeMap[joint.nodeIdx] ? nodeMap[joint.nodeIdx]->transform : nullptr;
				bones.push_back(bone);
			}
			setup.renderer->SetBones(std::move(bones));
		}
	}
	SH_EDITOR_API void PrefabHierarchy::OnHierarchyDraged(EditorWorld& world, core::SObject& payload)
	{
		game::Prefab& prefabPtr = static_cast<game::Prefab&>(payload);
		prefabPtr.AddToWorld(world);
	}
}//namespace