#include "Sound/WavLoader.h"

#include "fmt/format.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace
{
	struct ChunkHeader
	{
		std::array<char, 4> id{};
		std::uint32_t size = 0;
	};

	template<typename T>
	auto Read(std::ifstream& stream) -> T
	{
		T value{};
		stream.read(reinterpret_cast<char*>(&value), sizeof(T));
		if (!stream)
			throw std::runtime_error{ "Failed to read WAV file." };
		return value;
	}

	auto ReadChunkHeader(std::ifstream& stream) -> ChunkHeader
	{
		ChunkHeader header{};
		stream.read(header.id.data(), static_cast<std::streamsize>(header.id.size()));
		if (!stream)
			throw std::runtime_error{ "Failed to read WAV chunk header." };

		header.size = Read<std::uint32_t>(stream);
		return header;
	}

	auto IsChunkId(const std::array<char, 4>& id, const char(&expected)[5]) -> bool
	{
		return id[0] == expected[0] && id[1] == expected[1] && id[2] == expected[2] && id[3] == expected[3];
	}

	auto ToAudioFormat(std::uint16_t channels, std::uint16_t bitsPerSample) -> sh::sound::AudioFormat
	{
		if (channels == 1 && bitsPerSample == 8)
			return sh::sound::AudioFormat::Mono8;
		if (channels == 1 && bitsPerSample == 16)
			return sh::sound::AudioFormat::Mono16;
		if (channels == 2 && bitsPerSample == 8)
			return sh::sound::AudioFormat::Stereo8;
		if (channels == 2 && bitsPerSample == 16)
			return sh::sound::AudioFormat::Stereo16;

		throw std::runtime_error{ fmt::format("Unsupported WAV format. channels={}, bitsPerSample={}", channels, bitsPerSample) };
	}

	void SkipPaddingByte(std::ifstream& stream, std::uint32_t chunkSize)
	{
		if ((chunkSize & 1u) != 0u)
			stream.ignore(1);
	}
}

namespace sh::sound
{
	SH_SOUND_API auto WavLoader::Load(const std::filesystem::path& path) -> AudioData
	{
		std::ifstream file{ path, std::ios::binary };
		if (!file.is_open())
			throw std::runtime_error{ fmt::format("Failed to open wav file: {}", path.string()) };

		std::array<char, 4> riff{};
		file.read(riff.data(), static_cast<std::streamsize>(riff.size()));
		const std::uint32_t riffSize = Read<std::uint32_t>(file);
		std::array<char, 4> wave{};
		file.read(wave.data(), static_cast<std::streamsize>(wave.size()));
		if (!file)
			throw std::runtime_error{ fmt::format("Invalid wav header: {}", path.string()) };

		if (!IsChunkId(riff, "RIFF") || !IsChunkId(wave, "WAVE"))
			throw std::runtime_error{ fmt::format("Unsupported wav container: {}", path.string()) };

		AudioData result{};
		bool bFoundFmt = false;
		bool bFoundData = false;

		while (file && (!bFoundFmt || !bFoundData))
		{
			const ChunkHeader chunk = ReadChunkHeader(file);

			if (IsChunkId(chunk.id, "fmt "))
			{
				if (chunk.size < 16)
					throw std::runtime_error{ fmt::format("Invalid wav fmt chunk size: {}", path.string()) };

				const std::uint16_t audioFormat = Read<std::uint16_t>(file);
				const std::uint16_t channels = Read<std::uint16_t>(file);
				result.sampleRate = Read<std::uint32_t>(file);
				Read<std::uint32_t>(file);
				Read<std::uint16_t>(file);
				const std::uint16_t bitsPerSample = Read<std::uint16_t>(file);

				if (chunk.size > 16)
					file.ignore(static_cast<std::streamsize>(chunk.size - 16));

				SkipPaddingByte(file, chunk.size);

				if (audioFormat != 1)
					throw std::runtime_error{ fmt::format("Only PCM wav is supported: {}", path.string()) };

				result.format = ToAudioFormat(channels, bitsPerSample);
				bFoundFmt = true;
			}
			else if (IsChunkId(chunk.id, "data"))
			{
				result.samples.resize(chunk.size);
				file.read(reinterpret_cast<char*>(result.samples.data()), static_cast<std::streamsize>(chunk.size));
				if (!file)
					throw std::runtime_error{ fmt::format("Failed to read wav data chunk: {}", path.string()) };

				SkipPaddingByte(file, chunk.size);
				bFoundData = true;
			}
			else
			{
				file.ignore(static_cast<std::streamsize>(chunk.size));
				SkipPaddingByte(file, chunk.size);
			}
		}

		if (!bFoundFmt || !bFoundData)
			throw std::runtime_error{ fmt::format("Wav file is missing required chunks: {}", path.string()) };

		if (riffSize == 0)
			throw std::runtime_error{ fmt::format("Invalid wav size: {}", path.string()) };

		return result;
	}
}//namespace