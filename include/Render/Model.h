#pragma once
#include "Export.h"
#include "Mesh.h"

#include "Core/SObject.h"
#include "Core/SContainer.hpp"

#include <vector>
#include <memory>
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
			glm::mat4 modelMatrix = glm::mat4{ 1.0f };
			Mesh* mesh = nullptr;
			std::vector<std::unique_ptr<Node>> children;
		};
	private:
		std::unique_ptr<Node> rootNode;

		core::SVector<Mesh*> meshes;
	public:
		SH_RENDER_API Model();
		SH_RENDER_API ~Model();

		SH_RENDER_API void Destroy() override;

		SH_RENDER_API void AddMeshes(std::unique_ptr<Node>&& node);
		/// @brief 해당 모델의 모든 메쉬를 가져오는 함수.
		/// @return 메쉬 벡터
		SH_RENDER_API auto GetMeshes() const -> const core::SVector<Mesh*>&;
		/// @brief 루트 노드를 가져오는 함수.
		/// @return 루트 노드 포인터
		SH_RENDER_API auto GetRootNode() const -> const Node*;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};
}//namespace