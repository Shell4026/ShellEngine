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
		// 할당된 T의 영역을 재활용하는 메모리
		struct Block
		{
			Block* next;
		};
		// 실질적인 데이터
		class Buffer
		{
		private:
			using Byte = uint8_t;
			static constexpr std::size_t blockSize = (sizeof(T) > sizeof(Block)) ? sizeof(T) : sizeof(Block);
			std::array<Byte, blockSize * count> data;
		public:
			Buffer* const next;
		public:
			Buffer(Buffer* next) :
				next(next), data()
			{}
			Buffer(const Buffer& other) = delete;
			Buffer(Buffer&& other) noexcept :
				next(other.next), data(std::move(other.data))
			{
				other.next = nullptr;
			}
			auto operator=(const Buffer&) -> Buffer& = delete;
			auto operator=(Buffer&& other) noexcept -> Buffer&
			{
				data = std::move(other.data);
				next = other.next;

				other.next = nullptr;
				return *this;
			}

			auto GetBlock(std::size_t index) -> T*
			{
				return reinterpret_cast<T*>(&data[blockSize * index]);
			}
		};

		std::size_t allocatedSize = 0;
		Block* firstFreeBlock = nullptr;
		Buffer* firstBuffer = nullptr;
	public:
		MemoryPool()
		{
			firstBuffer = new Buffer(firstBuffer);
		}
		MemoryPool(const MemoryPool& other) :
			allocatedSize(other.allocatedSize),
			firstFreeBlock(nullptr),
			firstBuffer(nullptr)
		{
			operator=(other);
		}
		MemoryPool(MemoryPool&& other) noexcept :
			firstBuffer(other.firstBuffer),
			firstFreeBlock(other.firstFreeBlock),
			allocatedSize(other.allocatedSize)
		{
			other.firstBuffer = nullptr;
			other.firstFreeBlock = nullptr;
			other.allocatedSize = 0;
		}
		~MemoryPool()
		{
			while (firstBuffer)
			{
				Buffer* buffer = firstBuffer;
				firstBuffer = buffer->next;
				delete buffer;
			}
		}

		auto operator=(const MemoryPool& other) -> MemoryPool&;
		auto operator=(MemoryPool&& other) noexcept -> MemoryPool&;

		/// @brief 메모리 풀에서 메모리를 할당 받는다.
		/// 주의: 클래스를 할당 할 때 생성자가 자동으로 호출되지 않음
		/// @return 할당 받은 주소를 가르키는 포인터
		auto Allocate() -> T*;

		/// @brief 할당한 메모리를 풀에 반환한다.
		/// 주의: 클래스를 소멸 할 때 소멸자가 자동으로 호출되지 않음
		/// @param ptr 할당 받았던 포인터
		void DeAllocate(T* ptr);

		/// @brief 남은 할당 가능한 수를 반환.
		/// 
		/// 반환 후 할당 대기중인 블록 수는 포함되지 않음.
		/// @return 남은 공간
		auto GetFreeSize() const -> std::size_t
		{
			return count - allocatedSize;
		}

		/// @brief 반환 후 할당 대기중인 메모리 블록이 있는가?
		/// @return 있다면 true, 없다면 false
		bool HasFreeBlock() const
		{
			return firstFreeBlock;
		}

		bool operator==(const MemoryPool& other) const
		{
			return firstBuffer == other.firstBuffer;
		}
	};

	//source
	template<typename T, std::size_t count, bool fixed>
	inline auto MemoryPool<T, count, fixed>::Allocate() -> T*
	{
		// 이미 해제된 메모리가 존재하는 경우
		if (firstFreeBlock)
		{
			Block* block = firstFreeBlock;
			firstFreeBlock = block->next;
			return reinterpret_cast<T*>(block);
		}
		// 할당한 메모리가 꽉찬 경우
		if (allocatedSize >= count)
		{
			if constexpr (fixed)
				throw std::bad_alloc{};
			// 이전에 할당됐던 버퍼가 다음 퍼버가 되며 firstBuffer는 새로 할당한 버퍼가 된다.
			firstBuffer = new Buffer(firstBuffer);
			allocatedSize = 0;
		}
		return firstBuffer->GetBlock(allocatedSize++);
	}

	template<typename T, std::size_t count, bool fixed>
	inline void MemoryPool<T, count, fixed>::DeAllocate(T* ptr)
	{
		Block* block = reinterpret_cast<Block*>(ptr);
		block->next = firstFreeBlock;
		firstFreeBlock = block;
	}

	template<typename T, std::size_t count, bool fixed>
	inline auto MemoryPool<T, count, fixed>::operator=(const MemoryPool& other) -> MemoryPool&
	{
		allocatedSize = other.allocatedSize;

		if (other.firstBuffer == nullptr)
			return *this;

		// 버퍼 복사
		std::unordered_map<Buffer*, Buffer*> bufferMap; // freeBlock을 찾기 위해 필요

		std::stack<Buffer*> otherBuffers{}; // 스택에 넣어야 제일 끝 버퍼부터 복사 가능
		Buffer* otherBuffer = other.firstBuffer;
		while (otherBuffer)
		{
			otherBuffers.push(otherBuffer);
			otherBuffer = otherBuffer->next;
		}
		otherBuffer = nullptr;
		while (!otherBuffers.empty())
		{
			Buffer* newBuffer = new Buffer(otherBuffer);
			otherBuffer = otherBuffers.top();
			otherBuffers.pop();

			newBuffer->data = otherBuffer->data;

			firstBuffer = newBuffer;

			bufferMap[otherBuffer] = newBuffer;
		}
		// 빈 블록 목록 복사
		Block* currentOtherFreeBlock = other.firstFreeBlock;
		while (currentOtherFreeBlock)
		{
			Buffer* originalBuffer = other.firstBuffer;
			Buffer* newBuffer = nullptr;
			std::size_t blockIndex = 0;
			bool found = false;

			while (originalBuffer)
			{
				for (std::size_t i = 0; i < count; ++i)
				{
					T* blockPtr = originalBuffer->GetBlock(i);
					if (reinterpret_cast<T*>(currentOtherFreeBlock) == blockPtr)
					{
						newBuffer = bufferMap[originalBuffer];
						blockIndex = i;
						found = true;
						break;
					}
				}
				if (found)
					break;
				originalBuffer = originalBuffer->next;
			}

			if (found && newBuffer)
			{
				// 복사된 버퍼에서 해당 인덱스의 블록 가져오기
				T* newBlockPtr = newBuffer->GetBlock(blockIndex);
				Block* newBlock = reinterpret_cast<Block*>(newBlockPtr);

				newBlock->next = firstFreeBlock;
				firstFreeBlock = newBlock;
			}

			currentOtherFreeBlock = currentOtherFreeBlock->next;
		}
		return *this;
	}
	template<typename T, std::size_t count, bool fixed>
	inline auto MemoryPool<T, count, fixed>::operator=(MemoryPool&& other) noexcept -> MemoryPool&
	{
		firstBuffer = other.firstBuffer;
		firstFreeBlock = other.firstFreeBlock;
		allocatedSize = other.allocatedSize;

		other.firstBuffer = nullptr;
		other.firstFreeBlock = nullptr;
		other.allocatedSize = 0;

		return *this;
	}
}//namespace