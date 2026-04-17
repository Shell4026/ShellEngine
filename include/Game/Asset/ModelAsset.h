#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/Model.h"
#include "Render/Skeleton.h"

#include <vector>
namespace sh::game
{
	/// @brief 모델 에셋 클래스
	/// @brief 데이터 구조
	///	@brief 모델 헤더 | (노드 헤더 | 노드 데이터)... | (스켈레톤 헤더 | 스켈레톤 데이터)...
	class ModelAsset : public core::Asset
	{
		SASSET(ModelAsset, "mdel")
	public:
		SH_GAME_API ModelAsset();
		SH_GAME_API ModelAsset(const render::Model& model);
		SH_GAME_API ~ModelAsset();

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetData() const -> const std::vector<render::Model::Node>&;
		SH_GAME_API auto GetSkeletonData() const -> const std::vector<render::Skeleton>&;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "mdel";
	private:
		struct ModelHeader
		{
			uint64_t nodeCount = 0;
			uint64_t skeletonCount = 0;
		};
		struct NodeHeader
		{
			int nameSize = 0;
			int skeletonIdx = -1;
			int childrenCount = 0;
			glm::mat4 modelMatrix{ 1.f };
			std::array<uint32_t, 4> meshUUID;
		};
		struct SkeletonHeader
		{
			uint64_t jointCount = 0;
		};

		core::SObjWeakPtr<const render::Model> modelPtr;
		std::vector<render::Model::Node> nodeData;
		std::vector<render::Skeleton> skeletonData;
	};
}//namespace