#pragma once
#include "Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/ShaderPass.h"

namespace sh::game
{
	class ShaderPassAsset : public core::Asset
	{
		SASSET(ShaderPassAsset, "pass")
	private:
		core::SObjWeakPtr<const render::ShaderPass> passPtr;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		SH_GAME_API ShaderPassAsset();
		SH_GAME_API ShaderPassAsset(const render::ShaderPass& pass);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;
	};
}//namespace