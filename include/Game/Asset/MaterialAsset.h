#pragma once
#include "Export.h"

#include "Core/Asset.h"
#include "Core/ISerializable.h"

namespace sh::render
{
	class Material;
}
namespace sh::game
{
	class MaterialAsset : public core::Asset
	{
		SASSET(MaterialAsset, "mat")
	public:
		SH_GAME_API MaterialAsset();
		SH_GAME_API MaterialAsset(const render::Material& mat);
		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetMaterialData() const -> const core::Json&;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "mat";
	private:
		core::Json matData;
		const render::Material* matPtr = nullptr;
	};
}
