#pragma once
#include "Editor/Export.h"
#include "Editor/EditorCommand.h"

#include <memory>
#include <string>
#include <string_view>
#include <deque>

namespace sh::editor
{
	class CommandHistory
	{
	public:
		SH_EDITOR_API CommandHistory();
		SH_EDITOR_API ~CommandHistory();

		SH_EDITOR_API void Execute(std::unique_ptr<EditorCommand>&& command);
		SH_EDITOR_API void Undo();
		SH_EDITOR_API void Redo();
		SH_EDITOR_API void Clear();

		SH_EDITOR_API void BeginTransaction(std::string_view name);
		SH_EDITOR_API void EndTransaction();
		SH_EDITOR_API void CancelTransaction();

		SH_EDITOR_API void SetUndoLimit(uint32_t limit) { undoLimit = limit; }

		SH_EDITOR_API auto CanUndo() const -> bool;
		SH_EDITOR_API auto CanRedo() const -> bool;
		SH_EDITOR_API auto IsTransactionOpen() const -> bool;

		SH_EDITOR_API auto GetUndoName() const -> std::string_view;
		SH_EDITOR_API auto GetRedoName() const -> std::string_view;
		SH_EDITOR_API auto GetUndoLimit() const -> uint32_t { return undoLimit; }
	private:
		class CompositeCommand;

		void PushExecutedCommand(std::unique_ptr<EditorCommand>&& command);
		void ClearRedoStack();
	private:
		std::deque<std::unique_ptr<EditorCommand>> undoStack;
		std::vector<std::unique_ptr<EditorCommand>> redoStack;
		std::unique_ptr<CompositeCommand> currentTransaction;

		uint32_t undoLimit = 50;
	};
}//namespace