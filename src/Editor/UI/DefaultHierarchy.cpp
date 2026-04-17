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

		const auto& nodes = model.GetNodes();
		const auto& skeletons = model.GetSkeletons();

		if (nodes.empty())
			return;

		auto rootObj = world.AddGameObject(model.GetName().ToString());

		// node index -> 생성된 GameObject 매핑 (bone 연결 시 사용)
		std::vector<game::GameObject*> nodeMap(nodes.size(), nullptr);
		nodeMap[0] = rootObj;

		struct SkinnedSetup
		{
			game::SkinnedMeshRenderer* renderer;
			int skeletonIdx;
		};
		std::vector<SkinnedSetup> skinnedSetups;

		std::queue<std::pair<int, game::GameObject*>> nodeQ; // nodeIdx, parentObj
		for (int childIdx : nodes[0].childrenIdx)
			nodeQ.push({ childIdx, rootObj });

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