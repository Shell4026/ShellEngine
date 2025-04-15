#pragma once
#include "Export.h"
#include "Mesh.h"

#include "Core/SObject.h"

#include <vector>
#include <memory>
namespace sh::render
{
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
		std::vector<Mesh*> meshes;
	public:
		SH_RENDER_API Model();
		SH_RENDER_API ~Model();

		SH_RENDER_API void AddMeshes(std::unique_ptr<Node>&& node);
		SH_RENDER_API auto GetMeshes() const -> const std::vector<Mesh*>&;
		SH_RENDER_API auto GetNode() const -> const Node*;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};
}//namespace