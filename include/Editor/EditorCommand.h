#pragma once
#include "Editor/Export.h"

#include <memory>
#include <string>
#include <string_view>

namespace sh::editor
{
	class EditorCommand
	{
	public:
		SH_EDITOR_API virtual ~EditorCommand() = default;

		SH_EDITOR_API virtual void Execute() = 0;
		SH_EDITOR_API virtual void Undo() = 0;

		/// @brief 직전 커맨드와 병합 가능하면 true를 반환한다.
		SH_EDITOR_API virtual auto MergeWith(const EditorCommand& other) const -> bool { return false; }

		SH_EDITOR_API virtual auto GetName() const -> std::string_view = 0;
	};
}//namespace