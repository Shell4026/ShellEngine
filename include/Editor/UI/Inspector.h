#pragma once
#include "Editor/Export.h"

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
;	public:
		SH_EDITOR_API Inspector(EditorWorld& world);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		SH_EDITOR_API static auto GetIcon(std::string_view typeName) -> game::GUITexture*;
		SH_EDITOR_API static void RenderPropertiesCustomInspector(const core::reflection::STypeInfo& type, core::SObject& obj, int idx);
		SH_EDITOR_API static void RenderProperties(const core::reflection::STypeInfo& type, core::SObject& obj, int idx);
		SH_EDITOR_API static void RenderProperty(const core::reflection::Property& prop, core::SObject& owner, int idx);
		SH_EDITOR_API static void RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject& propertyOwner, const std::string& name,
			core::SObject** propertyPtr = nullptr, const core::reflection::TypeInfo* type = nullptr);
		SH_EDITOR_API static void RenderSObjPtrContainerProperty(const core::reflection::Property& prop, core::SObject& propertyOwner);
		SH_EDITOR_API static void RenderContainerProperty(const core::reflection::Property& prop, core::SObject& propertyOwner, const std::string& name);
	public:
		constexpr static const char* name = "Inspector";
		EditorWorld& world;
	private:
		CustomInspectorManager* customInspectorManager;
	};
}//namespace