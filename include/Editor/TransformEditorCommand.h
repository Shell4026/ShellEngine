#pragma once
#include "Editor/EditorCommand.h"

#include "Core/UUID.h"

#include "Game/Vector.h"

#include "glm/gtc/quaternion.hpp"

#include <string>
#include <string_view>

namespace sh::editor
{
	class TransformEditorCommand : public EditorCommand
	{
	public:
		struct Snapshot
		{
			game::Vec3 position;
			game::Vec3 scale;
			glm::quat rotation;
		};
	public:
		SH_EDITOR_API TransformEditorCommand(const core::UUID& objectUUID, std::string name, Snapshot before, Snapshot after);

		SH_EDITOR_API void Execute() override;
		SH_EDITOR_API void Undo() override;
		SH_EDITOR_API auto GetName() const -> std::string_view override;
	private:
		void Apply(const Snapshot& snapshot) const;
	private:
		core::UUID objectUUID;
		std::string name;
		Snapshot before;
		Snapshot after;
	};
}//namespace