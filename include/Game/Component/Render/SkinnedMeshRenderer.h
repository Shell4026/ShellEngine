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

	/// @brief 스켈레탈 메쉬 렌더러.
	///
	/// ### 설정 순서
	/// 1. SetSkinnedMesh()  — 스킨드 메쉬 설정
	/// 2. SetBones()        — 바인드 포즈 상태의 bone Transform 배열 설정 (IBM 자동 계산)
	///
	/// ### 매 프레임 동작
	/// ComputeBoneMatrices(): finalBoneMatrix[i] = bones[i]->localToWorldMatrix * inverseBindMatrices[i]
	/// → GPU BONES_UBO (set=1, binding=0) 업로드
	class SkinnedMeshRenderer : public MeshRenderer
	{
		COMPONENT(SkinnedMeshRenderer)
	public:
		SH_GAME_API SkinnedMeshRenderer(GameObject& owner);
		SH_GAME_API ~SkinnedMeshRenderer();

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;

		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API void SetSkinnedMesh(render::SkinnedMesh* mesh);
		SH_GAME_API void SetBones(std::vector<Transform*> bones);

		SH_GAME_API auto GetBones() const -> const std::vector<Transform*>& { return bones; }
		SH_GAME_API auto GetInverseBindMatrices() const -> const std::vector<glm::mat4>& { return inverseBindMatrices; }
	protected:
		SH_GAME_API void CreateDrawable() override;
		SH_GAME_API void UpdateDrawable() override;
	private:
		void ComputeBoneMatrices();
		void UploadBoneMatrices();
		PROPERTY(bones)
		std::vector<Transform*> bones;
		std::vector<glm::mat4> inverseBindMatrices;
		std::array<glm::mat4, render::SkinnedMesh::MAX_BONES> finalBoneMatrices;
	};
}//namespace
