#include "Editor/TransformEditorCommand.h"
#include "AssetDatabase.h"

#include "Core/Reflection.hpp"
#include "Core/SObjectManager.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	TransformEditorCommand::TransformEditorCommand(const core::UUID& objectUUID, std::string name, Snapshot before, Snapshot after) :
		objectUUID(objectUUID),
		name(std::move(name)),
		before(std::move(before)),
		after(std::move(after))
	{
	}

	SH_EDITOR_API void TransformEditorCommand::Execute()
	{
		Apply(after);
	}

	SH_EDITOR_API void TransformEditorCommand::Undo()
	{
		Apply(before);
	}

	SH_EDITOR_API auto TransformEditorCommand::GetName() const -> std::string_view
	{
		return name;
	}

	void TransformEditorCommand::Apply(const Snapshot& snapshot) const
	{
		auto* sobj = core::SObjectManager::GetInstance()->GetSObject(objectUUID);
		auto* obj = core::reflection::Cast<game::GameObject>(sobj);
		if (!core::IsValid(obj))
			return;

		obj->transform->SetWorldPosition(snapshot.position);
		obj->transform->SetWorldRotation(snapshot.rotation);
		obj->transform->SetScale(snapshot.scale);
	}
}//namespace