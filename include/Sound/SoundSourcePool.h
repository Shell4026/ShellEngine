#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace sh::sound
{
	class SoundClip;
	class SoundSource;

	class SoundSourcePool : public core::Singleton<SoundSourcePool>
	{
		friend core::Singleton<SoundSourcePool>;
	public:
		struct Handle
		{
			static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);

			uint32_t index = InvalidIndex;
			uint32_t generation = 0;

			auto IsValid() const -> bool
			{
				return index != InvalidIndex;
			}
		};
	public:
		SH_SOUND_API auto Acquire(const SoundClip* clip = nullptr) -> Handle;
		SH_SOUND_API void Release(Handle handle);
		SH_SOUND_API void Clear();

		SH_SOUND_API auto GetSource(Handle handle) -> SoundSource*;
		SH_SOUND_API auto GetSource(Handle handle) const -> const SoundSource*;
	protected:
		SH_SOUND_API SoundSourcePool();
		SH_SOUND_API ~SoundSourcePool();
	private:
		void CollectFinishedSources();
		auto IsHandleValid(Handle handle) const -> bool;
	private:
		struct PoolItem;
		std::vector<std::unique_ptr<PoolItem>> items;
		std::vector<uint32_t> freeIndices;
	};
}//namespace
