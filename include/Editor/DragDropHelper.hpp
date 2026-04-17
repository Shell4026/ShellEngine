#pragma once
#include "Core/SObject.h"
#include "Core/Reflection/STypeInfo.hpp"

#include "Game/ImGUImpl.h"

namespace sh::editor::dragdrop
{
	template<typename T>
	auto AcceptAsset() -> T*
	{
		const ImGuiPayload* current = ImGui::GetDragDropPayload();
		if (current == nullptr || !current->IsDataType("SObject"))
			return nullptr;

		auto* obj = *reinterpret_cast<core::SObject**>(current->Data);
		if (!obj->GetType().IsChildOf(T::GetStaticType()))
			return nullptr;

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SObject"))
			return static_cast<T*>(*reinterpret_cast<core::SObject**>(payload->Data));
		return nullptr;
	}

	inline auto AcceptAsset(const core::reflection::STypeInfo& type) -> core::SObject*
	{
		const ImGuiPayload* current = ImGui::GetDragDropPayload();
		if (current == nullptr || !current->IsDataType("SObject"))
			return nullptr;

		auto* obj = *reinterpret_cast<core::SObject**>(current->Data);
		if (!obj->GetType().IsChildOf(type))
			return nullptr;

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SObject"))
			return *reinterpret_cast<core::SObject**>(payload->Data);
		return nullptr;
	}

	inline auto AcceptAnyAsset() -> core::SObject*
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SObject"))
			return static_cast<core::SObject*>(*reinterpret_cast<core::SObject**>(payload->Data));
		return nullptr;
	}
}//namespace