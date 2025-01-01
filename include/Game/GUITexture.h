#pragma once

#include "Export.h"
#include "ImGUImpl.h"

#include "Core/NonCopyable.h"

namespace sh::render
{
	class IRenderContext;
	class Texture;
}
namespace sh::game
{
	/// @brief ImGUI에서 쓰는 텍스쳐
	class GUITexture : core::INonCopyable
	{
	private:
		const render::IRenderContext* context;

		ImTextureID tex;
	public:
		SH_GAME_API GUITexture();
		SH_GAME_API ~GUITexture();

		SH_GAME_API void Create(const render::IRenderContext& context, const render::Texture& texture);
		SH_GAME_API void Clean();
		SH_GAME_API bool IsValid() const;

		SH_GAME_API operator ImTextureID() const;
	};
}//namespace