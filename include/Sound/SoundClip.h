#pragma once
#include "Export.h"

#include "AudioData.h"
#include "SoundBuffer.h"

#include "Core/SObject.h"

#include <filesystem>
#include <memory>

namespace sh::sound
{
	class SoundClip : public core::SObject
	{
		SCLASS(SoundClip)
	public:
		SH_SOUND_API SoundClip();
		SH_SOUND_API ~SoundClip() override;

		SH_SOUND_API void SetAudioData(const AudioData& data);
		SH_SOUND_API void SetAudioData(AudioData&& data);
		SH_SOUND_API void LoadFromFile(const std::filesystem::path& path);

		SH_SOUND_API auto GetAudioData() const -> const AudioData& { return audioData; }
		SH_SOUND_API auto GetBuffer() const -> const SoundBuffer* { return buffer.get(); }
		SH_SOUND_API auto GetDuration() const -> float { return audioData.GetDuration(); }
	private:
		void RebuildBuffer();
	private:
		AudioData audioData;
		std::unique_ptr<SoundBuffer> buffer;
	};
}//namespace