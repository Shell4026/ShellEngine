#pragma once
#include "Export.h"

#include "AudioData.h"

#include <filesystem>

namespace sh::sound
{
	class OggLoader
	{
	public:
		SH_SOUND_API static auto Load(const std::filesystem::path& path) -> AudioData;
	};
}//namespace