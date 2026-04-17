#pragma once
#include "Export.h"
#include "Mesh.h"
#include "Skeleton.h"

#include "Core/SObject.h"
#include "Core/SContainer.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
namespace sh::render
{
	/// @brief 메쉬의 집합
	class Model : public core::SObject
	{
		SCLASS(Model)
	public:
		struct Node
		{
			std::string name;
			glm::mat4 modelMatrix{ 1.0f };
			Mesh* mesh = nullptr;
			int skeletonIdx = -1;
			std::vector<int> childrenIdx;
		};
	private:
		std::vector<Node> nodes;
		core::SVector<Mesh*> meshes;
		std::vector<Skeleton> skeletons;
	public:
		SH_RENDER_API Model(std::vector<Node> nodes);
		SH_RENDER_API ~Model();

		SH_RENDER_API void Destroy() override;

		SH_RENDER_API void SetSkeletons(std::vector<Skeleton> skeletons) { this->skeletons = std::move(skeletons); }

		SH_RENDER_API auto GetMeshes() const -> const core::SVector<Mesh*>& { return meshes; }
		SH_RENDER_API auto GetNodes() const -> const std::vector<Node>& { return nodes; }
		SH_RENDER_API auto GetSkeletons() const -> const std::vector<Skeleton>& { return skeletons; }

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};
}//namespace
