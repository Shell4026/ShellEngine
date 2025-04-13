#pragma once

#include "Core/ISerializable.h"

namespace sh::editor
{
	class IImporter : public core::ISerializable
	{
	public:
		virtual auto GetName() const -> const char* = 0;
	};
}