#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sh::sound
{
	class SoundSystem : public core::Singleton<SoundSystem>
	{
		friend core::Singleton<SoundSystem>;
	public:
		enum class DistanceModel
		{
			None,
			InverseDistance,
			InverseDistanceClamped,
			LinearDistance,
			LinearDistanceClamped,
			ExponentDistance,
			ExponentDistanceClamped
		};
	public:
		SH_SOUND_API void Init(std::string_view deviceName = {});
		SH_SOUND_API void Shutdown();

		SH_SOUND_API void SetListenerGain(float gain);
		SH_SOUND_API auto GetListenerGain() const -> float;
		SH_SOUND_API void SetListenerPosition(float x, float y, float z);
		SH_SOUND_API void SetListenerVelocity(float x, float y, float z);
		SH_SOUND_API void SetListenerOrientation(const std::array<float, 3>& at, const std::array<float, 3>& up);
		SH_SOUND_API void SetDistanceModel(DistanceModel model);

		SH_SOUND_API auto GetDeviceName() const -> std::string;
		SH_SOUND_API auto IsInit() const noexcept -> bool;

		SH_SOUND_API static auto GetDefaultDeviceName() -> std::string;
		SH_SOUND_API static auto EnumeratePlaybackDevices() -> std::vector<std::string>;
	protected:
		SH_SOUND_API SoundSystem();
		SH_SOUND_API ~SoundSystem();
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}//namespace