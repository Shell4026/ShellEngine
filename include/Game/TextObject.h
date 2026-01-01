#pragma once
#include "Export.h"

#include "Core/SObject.h"

namespace sh::game
{
	class TextObject : public core::SObject
	{
		SCLASS(TextObject)
	public:
		SH_GAME_API auto operator=(const TextObject& other) -> TextObject&;
		SH_GAME_API auto operator=(TextObject&& other) noexcept -> TextObject&;
	public:
		PROPERTY(text)
		std::string text;
	};
}//namespace