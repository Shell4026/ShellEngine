#pragma once
#include "Game/Export.h"
#include "MeshRenderer.h"
#include "DebugRenderer.h"

#include "Render/Font.h"

#include <string>
namespace sh::render
{
	class Mesh;
}
namespace sh::game
{
	class TextRenderer : public MeshRenderer
	{
		COMPONENT(TextRenderer)
	public:
		SH_GAME_API TextRenderer(GameObject& owner);

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetText(const std::string& text);
		SH_GAME_API void SetText(std::string&& text);

		SH_GAME_API auto GetText() const -> const std::string& { return txt; }
	private:
		void Setup();
		auto CreateQuad() -> render::Mesh*;
		void UpdateDebugRenderer();
	private:
		PROPERTY(txt)
		std::string txt;
		PROPERTY(font)
		render::Font* font = nullptr;
		PROPERTY(width)
		float width = 100.f;
		float height = 0.f;
		float beforeScaleX = 1.f;
		float beforeScaleY = 1.f;

		DebugRenderer* debugRenderer = nullptr;
		PROPERTY(bDisplayArea)
		bool bDisplayArea = true;
	};
}//namespace