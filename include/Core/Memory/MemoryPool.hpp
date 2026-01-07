#pragma once
#include "../NonCopyable.h"

#include <memory>
#include <type_traits>
#include <array>
#include <stack>
#include <unordered_map>

namespace sh::core::memory
{
	/// @brief 메모리 풀
	/// @tparam T 타입
	/// @tparam count 할당 갯수. 기본 128
	/// @tparam fixed true일 시 고정된 크기이며 꽉차도 확장되지 않음.
	template<typename T, std::size_t count = 128, bool fixed = false>
	class MemoryPool
	{
	private:
		/// @brief 할당된 T의 영역을 재활용하는 메모리
		struct Block
		{
			Block* next = nullptr;
		};
		/// @brief 실질적인 데이터
		class Buffer
		{
		public:
			Buffer(Buffer* next) :
				next(next), data()
			{}
			Buffer(const Buffer& other) = delete;
			auto operator=(const Buffer&) -> Buffer& = delete;

			auto GetBlock(std::size_t index) -> T* { return reinterpret_cast<T*>(&data[STRIDE * index]); }
		public:
			Buffer* const next;
		private:
			using Byte = uint8_t;
			static constexpr std::size_t BLOCK_SIZE = (sizeof(T) > sizeof(Block)) ? sizeof(T) : sizeof(Block);
			static constexpr std::size_t REQUIRED_ALIGNMENT = (alignof(T) > alignof(Block)) ? alignof(T) : alignof(Block);
			static constexpr std::size_t STRIDE = (BLOCK_SIZE + REQUIRED_ALIGNMENT - 1) & ~(REQUIRED_ALIGNMENT - 1); // 정렬값의 배수로 올림

			alignas(REQUIRED_ALIGNMENT) std::array<Byte, STRIDE * count> data;
		};
	public:
		MemoryPool()
		{
			curBuffer = new Buffer{ nullptr };
		}
		MemoryPool(const MemoryPool& other) = delete;
		MemoryPool(MemoryPool&& other) noexcept
		{
			operator=(std::move(other));
		}
		~MemoryPool()
		{
			Clear();
		}

		auto operator=(const MemoryPool& other) -> MemoryPool& = delete;
		auto operator=(MemoryPool&& other) noexcept -> MemoryPool&;
		void Clear();
		/// @brief 메모리 풀에서 메모리를 할당 받는다.
		/// @brief 주의: 클래스를 할당 할 때 생성자가 자동으로 호출되지 않음
		/// @return 할당 받은 주소를 가르키는 포인터
		auto Allocate() -> T*;
		/// @brief 할당한 메모리를 풀에 반환한다.
		/// @brief 주의: 클래스를 소멸 할 때 소멸자가 자동으로 호출되지 않음
		/// @param ptr 할당 받았던 포인터
		void DeAllocate(T* ptr);
		/// @brief 남은 할당 가능한 수를 반환.
		/// @return 남은 공간
		auto GetFreeSize() const -> std::size_t { return count - allocatedSize + freeSize; }

		/// @brief 반환 후 할당 대기중인 메모리 블록이 있는가?
		/// @return 있다면 true, 없다면 false
		auto HasFreeBlock() const -> bool { return curFreeBlock != nullptr; }
	private:
		Block* curFreeBlock = nullptr;
		Buffer* curBuffer = nullptr;
		std::size_t allocatedSize = 0;
		std::size_t freeSize = 0;
	};

	template<typename T, std::size_t count, bool fixed>
	inline auto MemoryPool<T, count, fixed>::operator=(MemoryPool&& other) noexcept -> MemoryPool&
	{
		Clear();
		curFreeBlock = other.curFreeBlock;
		curBuffer = other.curBuffer;
		allocatedSize = other.allocatedSize;
		freeSize = other.freeSize;
		other.curFreeBlock = nullptr;
		other.curBuffer = nullptr;
		return *this;
	}

	template<typename T, std::size_t count, bool fixed>
	inline void MemoryPool<T, count, fixed>::Clear()
	{
		Buffer* cur = curBuffer;
		while (cur != nullptr)
		{
			Buffer* next = cur->next;
			delete cur;
			cur = next;
		}
		curBuffer = nullptr;
		curFreeBlock = nullptr;
		allocatedSize = 0;
		freeSize = 0;
	}

	template<typename T, std::size_t count, bool fixed>
	inline auto MemoryPool<T, count, fixed>::Allocate() -> T*
	{
		// 이미 해제된 메모리가 존재하는 경우 재활용
		if (curFreeBlock != nullptr)
		{
			Block* freeBlock = curFreeBlock;
			curFreeBlock = curFreeBlock->next;
			--freeSize;
			return reinterpret_cast<T*>(freeBlock);
		}

		if (allocatedSize < count)
			return curBuffer->GetBlock(allocatedSize++);
		if constexpr (fixed)
			throw std::bad_alloc{};

		curBuffer = new Buffer{ curBuffer };
		allocatedSize = 0;
		return curBuffer->GetBlock(allocatedSize++);
	}

	template<typename T, std::size_t count, bool fixed>
	inline void MemoryPool<T, count, fixed>::DeAllocate(T* ptr)
	{
		Block* block = reinterpret_cast<Block*>(ptr);
		block->next = curFreeBlock;
		curFreeBlock = block;
		++freeSize;
	}
}//namespace