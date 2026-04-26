#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"
#include "Core/ISerializable.h"

#include "Render/ComputeShader.h"

namespace sh::game
{
	class ComputeShaderAsset : public core::Asset
	{
		SASSET(ComputeShaderAsset, "comp")
	public:
		constexpr static const char* ASSET_NAME = "comp";
	public:
		SH_GAME_API ComputeShaderAsset();
		SH_GAME_API ComputeShaderAsset(const render::ComputeShader& shader);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetShaderObjectJson() const -> const core::Json& { return shaderObjJson; }
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	private:
		core::SObjWeakPtr<const render::ComputeShader> shaderPtr;

		core::Json shaderObjJson;
	};
}//namespace
