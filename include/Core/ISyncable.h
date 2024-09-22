#pragma once

#include <array>

namespace sh::core
{
	/// @brief 0 = 게임 스레드, 1 = 렌더 스레드 데이터를 나타내는 배열 타입
	/// @tparam T 타입
	/// @tparam size 사이즈(기본 2)
	template<typename T, std::size_t size = 2>
	using SyncArray = std::array<T, size>;

	/// @brief 여러 스레드에서 접근 할 가능성이 있는 객체 인터페이스.
	class ISyncable
	{
	public:
		static constexpr int GAME_THREAD = 0;
		static constexpr int RENDER_THREAD = 1;
	public:
		virtual ~ISyncable() = default;

		/// @brief 데이터가 변했음을 알리는 함수.
		virtual void SetDirty() = 0;
		/// @brief 게임 스레드와 렌더 스레드의 데이터를 동기화 하는 함수. 두 스레드가 동기화 되는 타이밍에 호출해야 한다.
		virtual void Sync() = 0;
	};
}