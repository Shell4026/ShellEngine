#include "Sound/SoundClip.h"

#include "Sound/OggLoader.h"
#include "Sound/WavLoader.h"

#include <algorithm>
#include <cctype>

namespace sh::sound
{
	SH_SOUND_API SoundClip::SoundClip() = default;

	SH_SOUND_API SoundClip::~SoundClip() = default;

	SH_SOUND_API void SoundClip::SetAudioData(const AudioData& data)
	{
		audioData = data;
		RebuildBuffer();
	}

	SH_SOUND_API void SoundClip::SetAudioData(AudioData&& data)
	{
		audioData = std::move(data);
		RebuildBuffer();
	}

	SH_SOUND_API void SoundClip::LoadFromFile(const std::filesystem::path& path)
	{
		std::string ext = path.extension().u8string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char ch)
			{
				return static_cast<char>(std::tolower(ch));
			});

		if (ext == ".ogg")
			SetAudioData(OggLoader::Load(path));
		else
			SetAudioData(WavLoader::Load(path));
	}

	void SoundClip::RebuildBuffer()
	{
		if (audioData.Empty())
		{
			buffer.reset();
			return;
		}

		if (buffer == nullptr)
			buffer = std::make_unique<SoundBuffer>();

		buffer->SetData(audioData);
	}
}//namespace
