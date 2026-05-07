#pragma once
#include "Game/Export.h"
#include "Game/Component/Render/MeshRenderer.h"

#include "Render/SkinnedMesh.h"

#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace sh::game
{
	class Transform;

	/// @brief 스켈레탈 메쉬 렌더러
	class SkinnedMeshRenderer : public MeshRenderer
	{
		COMPONENT(SkinnedMeshRenderer)
	public:
		SH_GAME_API SkinnedMeshRenderer(GameObject& owner);
		SH_GAME_API ~SkinnedMeshRenderer();

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetSkinnedMesh(render::SkinnedMesh* mesh);
		SH_GAME_API void SetBones(std::vector<Transform*> bones);

		SH_GAME_API auto GetBones() const -> const std::vector<Transform*>& { return bones; }
		SH_GAME_API auto GetInverseBindMatrices() const -> const std::vector<glm::mat4>& { return inverseBindMatrices; }
	protected:
		SH_GAME_API void UpdateDrawable() override;
	private:
		void ComputeBoneMatrices();
		void UploadBoneMatrices();
		void InitIBM();
	private:
		PROPERTY(bones, core::PropertyOption::invisible)
		std::vector<Transform*> bones;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<glm::mat4> finalBoneMatrices;
	};
}//namespace
