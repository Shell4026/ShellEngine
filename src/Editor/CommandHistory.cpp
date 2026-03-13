#include "Editor/CommandHistory.h"

#include <utility>

namespace sh::editor
{
	class CommandHistory::CompositeCommand : public EditorCommand
	{
	public:
		explicit CompositeCommand(std::string name) :
			name(std::move(name))
		{
		}

		void Execute() override
		{
			for (auto& command : commands)
				command->Execute();
		}

		void Undo() override
		{
			for (auto it = commands.rbegin(); it != commands.rend(); ++it)
				(*it)->Undo();
		}

		auto GetName() const -> std::string_view override
		{
			return name;
		}

		auto Empty() const -> bool
		{
			return commands.empty();
		}

		void Add(std::unique_ptr<EditorCommand>&& command)
		{
			if (command != nullptr)
				commands.push_back(std::move(command));
		}
	private:
		std::string name;
		std::vector<std::unique_ptr<EditorCommand>> commands;
	};

	CommandHistory::CommandHistory()
	{
	}
	CommandHistory::~CommandHistory()
	{
	}

	SH_EDITOR_API void CommandHistory::Execute(std::unique_ptr<EditorCommand>&& command)
	{
		if (command == nullptr)
			return;

		command->Execute();

		if (currentTransaction != nullptr)
		{
			currentTransaction->Add(std::move(command));
			ClearRedoStack();
			return;
		}

		PushExecutedCommand(std::move(command));
	}

	SH_EDITOR_API void CommandHistory::Undo()
	{
		if (!CanUndo())
			return;

		std::unique_ptr<EditorCommand> command = std::move(undoStack.back());
		undoStack.pop_back();

		command->Undo();
		redoStack.push_back(std::move(command));
	}

	SH_EDITOR_API void CommandHistory::Redo()
	{
		if (!CanRedo())
			return;

		std::unique_ptr<EditorCommand> command = std::move(redoStack.back());
		redoStack.pop_back();

		command->Execute();
		undoStack.push_back(std::move(command));
	}

	SH_EDITOR_API void CommandHistory::Clear()
	{
		undoStack.clear();
		redoStack.clear();
		currentTransaction.reset();
	}

	SH_EDITOR_API void CommandHistory::BeginTransaction(std::string_view name)
	{
		if (currentTransaction != nullptr)
			return;

		currentTransaction = std::make_unique<CompositeCommand>(std::string{ name });
	}

	SH_EDITOR_API void CommandHistory::EndTransaction()
	{
		if (currentTransaction == nullptr)
			return;

		if (currentTransaction->Empty())
		{
			currentTransaction.reset();
			return;
		}

		PushExecutedCommand(std::move(currentTransaction));
	}

	SH_EDITOR_API void CommandHistory::CancelTransaction()
	{
		if (currentTransaction == nullptr)
			return;

		currentTransaction->Undo();
		currentTransaction.reset();
	}

	SH_EDITOR_API auto CommandHistory::CanUndo() const -> bool
	{
		return !undoStack.empty();
	}

	SH_EDITOR_API auto CommandHistory::CanRedo() const -> bool
	{
		return !redoStack.empty();
	}

	SH_EDITOR_API auto CommandHistory::IsTransactionOpen() const -> bool
	{
		return currentTransaction != nullptr;
	}

	SH_EDITOR_API auto CommandHistory::GetUndoName() const -> std::string_view
	{
		if (!CanUndo())
			return {};
		return undoStack.back()->GetName();
	}

	SH_EDITOR_API auto CommandHistory::GetRedoName() const -> std::string_view
	{
		if (!CanRedo())
			return {};
		return redoStack.back()->GetName();
	}

	void CommandHistory::PushExecutedCommand(std::unique_ptr<EditorCommand>&& command)
	{
		if (command == nullptr)
			return;

		ClearRedoStack();

		if (!undoStack.empty() && undoStack.back()->MergeWith(*command))
			return;

		if (undoStack.size() > undoLimit)
			undoStack.pop_front();
		undoStack.push_back(std::move(command));
	}

	void CommandHistory::ClearRedoStack()
	{
		redoStack.clear();
	}
}//namespace