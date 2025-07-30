#pragma once
#include "Game/Export.h"
#include "Core/IAssetLoader.h"

namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class ComponentModule;
	class ImGUImpl;
	class World;
	class WorldLoader : public core::IAssetLoader
	{
	private:
		render::Renderer& renderer;
		const ComponentModule& componentModule;
		ImGUImpl& guiContext;
	public:
		SH_GAME_API WorldLoader(render::Renderer& renderer, ImGUImpl& guiContext);
		SH_GAME_API auto Load(const std::filesystem::path& path) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}
