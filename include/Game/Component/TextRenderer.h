#pragma once
#include "Export.h"
#include "Component/MeshRenderer.h"

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

		SH_GAME_API void Start() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetText(const std::string& text);
		SH_GAME_API void SetText(std::string&& text);

		SH_GAME_API auto GetText() const -> const std::string& { return txt; }
	private:
		void Setup();
		auto CreateQuad() -> render::Mesh*;
	private:
		PROPERTY(txt)
		std::string txt;
		PROPERTY(font)
		render::Font* font = nullptr;
	};
}//namespace