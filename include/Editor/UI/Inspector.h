#pragma once
#include "Export.h"

#include "Core/Reflection.hpp"

#include <unordered_map>
#include <vector>
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
	class CustomInspectorManager;

	class Inspector
	{
	public:
		EditorWorld& world;

		constexpr static const char* name = "Inspector";
	private:
		auto GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>;
		void RenderAddComponent(game::GameObject& gameObject);
		void RenderProperties(const core::reflection::STypeInfo& type, core::SObject& obj, int idx);
		void RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject& propertyOwner, const std::string& name, 
			core::SObject** propertyPtr = nullptr, const core::reflection::TypeInfo* type = nullptr);
		void RenderSObjPtrContainerProperty(const core::reflection::Property& prop, core::SObject& propertyOwner);
		void RenderContainerProperty(const core::reflection::Property& prop, core::SObject& obj, const std::string& name);
;	public:
		SH_EDITOR_API Inspector(EditorWorld& world);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		SH_EDITOR_API static auto GetIcon(std::string_view typeName) -> const game::GUITexture*;
		SH_EDITOR_API static void RenderProperty(const core::reflection::Property& prop, core::SObject& owner, int idx);
	private:
		CustomInspectorManager* customInspectorManager;

		std::unordered_map<std::string, std::vector<std::string>> componentItems;

		bool bAddComponent = false;
	};
}//namespace