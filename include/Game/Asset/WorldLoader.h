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
		/// @brief 월드 에셋을 불러온다. 불러온 월드의 리소스는 이 시점에 메모리에 올라가지 않는다.
		/// @param path 경로
		SH_GAME_API auto Load(const std::filesystem::path& path) -> core::SObject* override;
		/// @brief 월드 에셋을 불러온다. 불러온 월드의 리소스는 이 시점에 메모리에 올라가지 않는다.
		/// @param asset 에셋
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}
