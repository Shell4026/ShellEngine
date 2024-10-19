#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/SContainer.hpp"

namespace sh::game
{
	class GUITexture;
	class World;
	class GameObject;
	class Component;
}
namespace sh::editor
{
	class EditorWorld;

	class Inspector : public UI
	{
	private:
		EditorWorld& world;

		core::SMap<std::string, core::SVector<std::string>> componentItems;

		bool bAddComponent = false;
	public:
		constexpr static const char* name = "Inspector";
	private:
		inline auto GetIcon(std::string_view typeName) const -> const game::GUITexture*;
		inline auto GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>;
		inline void RenderAddComponent(game::GameObject& gameObject);
		inline void RenderSObjectPtrProperty(const core::reflection::Property& prop, game::Component* component, const std::string& name, const std::string& typeName);
;	public:
		SH_EDITOR_API Inspector(game::ImGUImpl& imgui, EditorWorld& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}//namespace