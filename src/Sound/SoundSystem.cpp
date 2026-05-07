#include "Sound/SoundSystem.h"

#include "Core/Logger.h"

#include "Sound/SoundSourcePool.h"

#include "fmt/format.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <stdexcept>
namespace
{
	auto GetAlErrorString(ALenum errorCode) -> const char*
	{
		switch (errorCode)
		{
		case AL_NO_ERROR:
			return "AL_NO_ERROR";
		case AL_INVALID_NAME:
			return "AL_INVALID_NAME";
		case AL_INVALID_ENUM:
			return "AL_INVALID_ENUM";
		case AL_INVALID_VALUE:
			return "AL_INVALID_VALUE";
		case AL_INVALID_OPERATION:
			return "AL_INVALID_OPERATION";
		case AL_OUT_OF_MEMORY:
			return "AL_OUT_OF_MEMORY";
		default:
			return "AL_UNKNOWN_ERROR";
		}
	}

	auto GetAlcErrorString(ALCenum errorCode) -> const char*
	{
		switch (errorCode)
		{
		case ALC_NO_ERROR:
			return "ALC_NO_ERROR";
		case ALC_INVALID_DEVICE:
			return "ALC_INVALID_DEVICE";
		case ALC_INVALID_CONTEXT:
			return "ALC_INVALID_CONTEXT";
		case ALC_INVALID_ENUM:
			return "ALC_INVALID_ENUM";
		case ALC_INVALID_VALUE:
			return "ALC_INVALID_VALUE";
		case ALC_OUT_OF_MEMORY:
			return "ALC_OUT_OF_MEMORY";
		default:
			return "ALC_UNKNOWN_ERROR";
		}
	}

	void ThrowIfAlError(std::string_view operation)
	{
		const ALenum errorCode = alGetError();
		if (errorCode != AL_NO_ERROR)
			throw std::runtime_error{ fmt::format("OpenAL error while {}: {}", operation, GetAlErrorString(errorCode)) };
	}

	void ThrowIfAlcError(ALCdevice* device, std::string_view operation)
	{
		const ALCenum errorCode = alcGetError(device);
		if (errorCode != ALC_NO_ERROR)
			throw std::runtime_error{ fmt::format("OpenAL context error while {}: {}", operation, GetAlcErrorString(errorCode)) };
	}

	auto ParseDeviceList(const ALCchar* deviceNames) -> std::vector<std::string>
	{
		std::vector<std::string> result;
		if (deviceNames == nullptr)
			return result;

		const ALCchar* current = deviceNames;
		while (*current != '\0')
		{
			std::string name{ current };
			result.push_back(name);
			current += name.size() + 1;
		}

		return result;
	}

	auto ToNativeDistanceModel(sh::sound::SoundSystem::DistanceModel model) -> ALenum
	{
		switch (model)
		{
		case sh::sound::SoundSystem::DistanceModel::None:
			return AL_NONE;
		case sh::sound::SoundSystem::DistanceModel::InverseDistance:
			return AL_INVERSE_DISTANCE;
		case sh::sound::SoundSystem::DistanceModel::InverseDistanceClamped:
			return AL_INVERSE_DISTANCE_CLAMPED;
		case sh::sound::SoundSystem::DistanceModel::LinearDistance:
			return AL_LINEAR_DISTANCE;
		case sh::sound::SoundSystem::DistanceModel::LinearDistanceClamped:
			return AL_LINEAR_DISTANCE_CLAMPED;
		case sh::sound::SoundSystem::DistanceModel::ExponentDistance:
			return AL_EXPONENT_DISTANCE;
		case sh::sound::SoundSystem::DistanceModel::ExponentDistanceClamped:
			return AL_EXPONENT_DISTANCE_CLAMPED;
		}
		return AL_INVERSE_DISTANCE_CLAMPED;
	}
}

namespace sh::sound
{
	struct SoundSystem::Impl
	{
		ALCdevice* device = nullptr;
		ALCcontext* context = nullptr;
	};

	SH_SOUND_API SoundSystem::SoundSystem() :
		impl(std::make_unique<Impl>())
	{
	}

	SH_SOUND_API SoundSystem::~SoundSystem()
	{
		Shutdown();
	}

	SH_SOUND_API void SoundSystem::Init(std::string_view deviceName)
	{
		if (!impl)
			impl = std::make_unique<Impl>();

		if (IsInit())
			return;

		std::string requestedDevice{ deviceName };
		impl->device = alcOpenDevice(requestedDevice.empty() ? nullptr : requestedDevice.c_str());
		if (impl->device == nullptr)
		{
			// 아마도 기본 사운드 시스템이 없는 경우임
			const ALenum err = alcGetError(nullptr);
			SH_ERROR_FORMAT("Failed to open OpenAL device: {}", GetAlcErrorString(err));
			return;
		}

		impl->context = alcCreateContext(impl->device, nullptr);
		if (impl->context == nullptr)
		{
			alcCloseDevice(impl->device);
			impl->device = nullptr;
			throw std::runtime_error{ "Failed to create OpenAL context." };
		}

		if (alcMakeContextCurrent(impl->context) == ALC_FALSE)
		{
			alcDestroyContext(impl->context);
			alcCloseDevice(impl->device);
			impl->context = nullptr;
			impl->device = nullptr;
			throw std::runtime_error{ "Failed to activate OpenAL context." };
		}

		try
		{
			ThrowIfAlcError(impl->device, "initializing sound system");
			alGetError();

			alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
			ThrowIfAlError("setting default distance model");
			alListenerf(AL_GAIN, 1.f);
			ThrowIfAlError("setting default listener gain");

			SH_INFO_FORMAT("OpenAL initialized. Device: {}", GetDeviceName());
		}
		catch (...)
		{
			Shutdown();
			throw;
		}
	}

	SH_SOUND_API void SoundSystem::Shutdown()
	{
		if (!impl)
			return;

		SoundSourcePool::Destroy();

		if (impl->context != nullptr)
		{
			alcMakeContextCurrent(nullptr);
			alcDestroyContext(impl->context);
			impl->context = nullptr;
		}

		if (impl->device != nullptr)
		{
			alcCloseDevice(impl->device);
			impl->device = nullptr;
		}
	}

	SH_SOUND_API void SoundSystem::SetListenerGain(float gain)
	{
		if (gain < 0.f)
			throw std::invalid_argument{ "Listener gain must be greater than or equal to zero." };

		alListenerf(AL_GAIN, gain);
		ThrowIfAlError("setting listener gain");
	}

	SH_SOUND_API auto SoundSystem::GetListenerGain() const -> float
	{
		float gain = 0.f;
		alGetListenerf(AL_GAIN, &gain);
		ThrowIfAlError("querying listener gain");
		return gain;
	}

	SH_SOUND_API void SoundSystem::SetListenerPosition(float x, float y, float z)
	{
		alListener3f(AL_POSITION, x, y, z);
		ThrowIfAlError("setting listener position");
	}

	SH_SOUND_API void SoundSystem::SetListenerVelocity(float x, float y, float z)
	{
		alListener3f(AL_VELOCITY, x, y, z);
		ThrowIfAlError("setting listener velocity");
	}

	SH_SOUND_API void SoundSystem::SetListenerOrientation(const std::array<float, 3>& at, const std::array<float, 3>& up)
	{
		const std::array<float, 6> orientation{ at[0], at[1], at[2], up[0], up[1], up[2] };
		alListenerfv(AL_ORIENTATION, orientation.data());
		ThrowIfAlError("setting listener orientation");
	}

	SH_SOUND_API void SoundSystem::SetDistanceModel(DistanceModel model)
	{
		alDistanceModel(ToNativeDistanceModel(model));
		ThrowIfAlError("setting distance model");
	}

	SH_SOUND_API auto SoundSystem::GetDeviceName() const -> std::string
	{
		if (!impl || impl->device == nullptr)
			return {};

		const ALCchar* name = alcGetString(impl->device, ALC_DEVICE_SPECIFIER);
		if (name == nullptr)
			return {};

		return std::string{ name };
	}

	SH_SOUND_API auto SoundSystem::GetListenerPosition() const -> std::array<float, 3>
	{
		float x, y, z;
		alGetListener3f(AL_POSITION, &x, &y, &z);
		return { x, y, z };
	}
	SH_SOUND_API auto SoundSystem::GetListenerForward() const -> std::array<float, 3>
	{
		float ori[6];
		alGetListenerfv(AL_ORIENTATION, ori);
		return { ori[0], ori[1], ori[2] };
	}
	SH_SOUND_API auto SoundSystem::GetListenerUp() const -> std::array<float, 3>
	{
		float ori[6];
		alGetListenerfv(AL_ORIENTATION, ori);
		return { ori[3], ori[4], ori[5] };
	}

	SH_SOUND_API auto SoundSystem::IsInit() const noexcept -> bool
	{
		return impl != nullptr && impl->context != nullptr;
	}

	SH_SOUND_API auto SoundSystem::GetDefaultDeviceName() -> std::string
	{
		const ALCchar* name = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		if (name == nullptr)
			return {};

		return std::string{ name };
	}

	SH_SOUND_API auto SoundSystem::EnumeratePlaybackDevices() -> std::vector<std::string>
	{
		if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT") == ALC_FALSE)
			return {};

		const ALCchar* deviceNames = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
		return ParseDeviceList(deviceNames);
	}
}//namespace
