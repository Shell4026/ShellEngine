#include "Sound/SoundSourcePool.h"

#include "Sound/SoundClip.h"
#include "Sound/SoundSource.h"

namespace sh::sound
{
	struct SoundSourcePool::PoolItem
	{
		PoolItem() :
			source(),
			onClipDestroyListener()
		{
			onClipDestroyListener.SetCallback(
				[this](const core::SObject*)
				{
					source.ClearBuffer();
					clip = nullptr;
				}
			);
		}

		void BindClip(const SoundClip* newClip)
		{
			UnbindClip();
			clip = newClip;

			if (clip != nullptr)
				clip->onDestroy.Register(onClipDestroyListener);
		}

		void UnbindClip()
		{
			if (clip != nullptr)
				clip->onDestroy.UnRegister(onClipDestroyListener);

			clip = nullptr;
		}

		SoundSource source;
		core::Observer<false, const core::SObject*>::Listener onClipDestroyListener;
		const SoundClip* clip = nullptr;
		uint32_t generation = 1;
		bool bInUse = false;
	};

	SH_SOUND_API SoundSourcePool::SoundSourcePool() = default;

	SH_SOUND_API SoundSourcePool::~SoundSourcePool()
	{
		Clear();
	}

	SH_SOUND_API auto SoundSourcePool::Acquire(const SoundClip* clip) -> Handle
	{
		CollectFinishedSources();

		uint32_t index = 0;
		if (freeIndices.empty())
		{
			index = static_cast<uint32_t>(items.size());
			items.push_back(std::make_unique<PoolItem>());
		}
		else
		{
			index = freeIndices.back();
			freeIndices.pop_back();
		}

		PoolItem& item = *items[index];
		item.source.ClearBuffer();
		item.BindClip(clip);
		item.bInUse = true;

		return Handle{ index, item.generation };
	}

	SH_SOUND_API void SoundSourcePool::Release(Handle handle)
	{
		if (!IsHandleValid(handle))
			return;

		PoolItem& item = *items[handle.index];
		item.UnbindClip();
		item.source.ClearBuffer();
		item.bInUse = false;
		++item.generation;
		freeIndices.push_back(handle.index);
	}

	SH_SOUND_API void SoundSourcePool::Clear()
	{
		for (auto& item : items)
		{
			if (item != nullptr)
				item->UnbindClip();
		}

		freeIndices.clear();
		items.clear();
	}

	SH_SOUND_API auto SoundSourcePool::GetSource(Handle handle) -> SoundSource*
	{
		if (!IsHandleValid(handle))
			return nullptr;

		return &items[handle.index]->source;
	}

	SH_SOUND_API auto SoundSourcePool::GetSource(Handle handle) const -> const SoundSource*
	{
		if (!IsHandleValid(handle))
			return nullptr;

		return &items[handle.index]->source;
	}

	void SoundSourcePool::CollectFinishedSources()
	{
		for (uint32_t i = 0; i < items.size(); ++i)
		{
			const auto& item = items[i];
			if (item == nullptr || !item->bInUse)
				continue;

			if (item->source.GetState() == SoundState::Stopped)
				Release(Handle{ i, item->generation });
		}
	}

	auto SoundSourcePool::IsHandleValid(Handle handle) const -> bool
	{
		if (!handle.IsValid() || handle.index >= items.size())
			return false;

		const auto& item = items[handle.index];
		if (item == nullptr || !item->bInUse)
			return false;

		return item->generation == handle.generation;
	}
}//namespace
