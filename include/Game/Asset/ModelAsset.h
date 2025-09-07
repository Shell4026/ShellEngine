#pragma once
#include "Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/Model.h"

#include <vector>
namespace sh::game
{
	/// @brief 모델 에셋 클래스
	/// @brief 데이터 구조
	///	@brief 모델 헤더 | (메쉬 헤더 | 메쉬 데이터(버텍스, 인덱스)) | 노드 헤더
	class ModelAsset : public core::Asset
	{
		SASSET(ModelAsset, "mdel")
	public:
		struct ModelHeader
		{
			uint64_t meshCount;
			uint64_t nodeCount;
		};
		struct MeshHeader
		{
			std::array<uint32_t, 4> uuid;
			uint64_t vertexCount;
			uint64_t indexCount;
		};
		struct NodeHeader
		{
			glm::mat4 modelMatrix;
			int32_t meshIndex;   // 메쉬가 없는 경우 -1
			int32_t parentIndex; // 루트인 경우 -1
			char name[128];
		};
		struct ModelData
		{
			const uint8_t* dataPtr;
			std::size_t size;
		};
	private:
		core::SObjWeakPtr<const render::Model> modelPtr;
		ModelHeader header;
	private:
		auto SetHeader(ModelHeader& header) const -> std::vector<NodeHeader>;
	public:
		constexpr static const char* ASSET_NAME = "mdel";
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		SH_GAME_API ModelAsset();
		SH_GAME_API ModelAsset(const render::Model& model);
		SH_GAME_API ~ModelAsset();

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetHeader() const -> ModelHeader;
		SH_GAME_API auto GetData() const -> ModelData;
	};
}//namespace