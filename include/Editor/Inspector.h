#pragma once

#include "Export.h"
#include "UI.h"

#include "Core/Reflection.hpp"
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
		inline void RenderProperties(const core::reflection::STypeInfo* type, core::SObject* obj, int idx);
		inline void RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject* propertyOwner, const std::string& name, 
			core::SObject** propertyPtr = nullptr, const core::reflection::TypeInfo* type = nullptr);
		inline void RenderContainerProperty(const core::reflection::Property& prop, core::SObject* obj, const std::string& name);
;	public:
		SH_EDITOR_API Inspector(game::ImGUImpl& imgui, EditorWorld& world);

		SH_EDITOR_API void Update() override;
		SH_EDITOR_API void Render() override;
	};
}//namespace