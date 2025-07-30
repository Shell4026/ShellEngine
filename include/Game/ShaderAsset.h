#pragma once
#include "Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"
#include "Core/ISerializable.h"

#include "Render/Shader.h"

namespace sh::game
{
	class ShaderAsset : public core::Asset
	{
		SASSET(ShaderAsset, "shad")
	private:
		core::SObjWeakPtr<const render::Shader> shaderPtr;

		core::Json shaderObjJson;
	public:
		constexpr static const char* ASSET_NAME = "shad";
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		SH_GAME_API ShaderAsset();
		SH_GAME_API ShaderAsset(const render::Shader& shader);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetShaderObjectJson() const -> const core::Json&;
	};
}//namespace