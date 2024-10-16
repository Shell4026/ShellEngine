#pragma once

#include "Export.h"
#include "UI.h"

namespace sh::game
{
	class World;
}
namespace sh::editor
{
	class EditorWorld;

	class Inspector : public UI
	{
	public:
		constexpr static const char* name = "Inspector";
		EditorWorld& world;

		bool bAddComponent = false;
;	public:
		SH_EDITOR_API Inspector(game::ImGUImpl& imgui, EditorWorld& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}//namespace