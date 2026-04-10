#pragma once
#include "Editor/EditorCommand.h"

#include <functional>
#include <utility>

namespace sh::editor
{
	class LambdaEditorCommand : public EditorCommand
	{
	public:
		LambdaEditorCommand(std::string name, std::function<void()> execute, std::function<void()> undo) :
			name(std::move(name)),
			execute(std::move(execute)),
			undo(std::move(undo))
		{
		}

		void Execute() override
		{
			if (execute)
				execute();
		}

		void Undo() override
		{
			if (undo)
				undo();
		}

		auto GetName() const -> std::string_view override
		{
			return name;
		}
	private:
		std::string name;
		std::function<void()> execute;
		std::function<void()> undo;
	};
}//namespace