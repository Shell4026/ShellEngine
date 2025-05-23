﻿#pragma once
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
	private:
		CustomInspectorManager* customInspectorManager;

		std::unordered_map<std::string, std::vector<std::string>> componentItems;

		bool bAddComponent = false;
	public:
		EditorWorld& world;

		constexpr static const char* name = "Inspector";
	private:
		inline auto GetComponentGroupAndName(std::string_view fullname) -> std::pair<std::string, std::string>;
		inline void RenderAddComponent(game::GameObject& gameObject);
		inline void RenderProperties(const core::reflection::STypeInfo* type, core::SObject* obj, int idx);
		inline void RenderSObjectPtrProperty(const core::reflection::Property& prop, core::SObject* propertyOwner, const std::string& name, 
			core::SObject** propertyPtr = nullptr, const core::reflection::TypeInfo* type = nullptr);
		inline void RenderContainerProperty(const core::reflection::Property& prop, core::SObject* obj, const std::string& name);
;	public:
		SH_EDITOR_API Inspector(EditorWorld& world);

		SH_EDITOR_API void Update();
		SH_EDITOR_API void Render();

		SH_EDITOR_API static auto GetIcon(std::string_view typeName) -> const game::GUITexture*;
	};
}//namespace