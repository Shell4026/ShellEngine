#pragma once

#include "../NonCopyable.h"

#include <memory>
#include <type_traits>
#include <array>

namespace sh::core::memory
{
	/// @brief 메모리 풀
	/// @tparam T 타입
	/// @tparam count 할당 갯수. 기본 128
	/// @tparam fixed true일 시 고정된 크기이며 꽉차도 확장되지 않음.
	template<typename T, std::size_t count = 128, bool fixed = false>
	class MemoryPool : public core::INonCopyable
	{
	private:
		//할당된 T의 영역을 재활용하는 메모리
		struct Block
		{
			Block* next;
		};
		//실질적인 데이터
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
			Buffer(Buffer&& other) noexcept :
				next(other.next), data(std::move(other.data))
			{
				other.next = nullptr;
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

		/// @brief 메모리 풀에서 메모리를 할당 받는다.
		/// 
		/// 주의: 클래스를 할당 할 때 생성자가 자동으로 호출되지 않음
		/// @return 할당 받은 주소를 가르키는 포인터
		auto Allocate() -> T*
		{
			//이미 해제된 메모리가 존재하는 경우
			if (firstFreeBlock)
			{
				Block* block = firstFreeBlock;
				firstFreeBlock = block->next;
				return reinterpret_cast<T*>(block);
			}
			//할당한 메모리가 꽉찬 경우
			if (allocatedSize >= count)
			{
				if constexpr (fixed)
					throw std::bad_alloc{};
				//이전에 할당됐던 버퍼가 다음 퍼버가 되며 firstBuffer는 새로 할당한 버퍼가 된다.
				firstBuffer = new Buffer(firstBuffer);
				allocatedSize = 0;
			}
			return firstBuffer->GetBlock(allocatedSize++);
		}

		/// @brief 할당한 메모리를 풀에 반환한다.
		/// 
		/// 주의: 클래스를 소멸 할 때 소멸자가 자동으로 호출되지 않음
		/// @param ptr 할당 받았던 포인터
		void DeAllocate(T* ptr)
		{
			Block* block = reinterpret_cast<Block*>(ptr);
			block->next = firstFreeBlock;
			firstFreeBlock = block;
		}

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
	};
}//namespace