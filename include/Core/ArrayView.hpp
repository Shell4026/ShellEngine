#pragma once

#include <cstdint>
namespace sh::core
{
	/// @brief std::string_view 처럼 배열 임의의 위치를 가르키는 객체
	/// @tparam T 타입
	template<typename T>
	class ArrayView
	{
	private:
		const T* ptr;
		std::size_t len;
	public:
		ArrayView(const T* ptr, std::size_t len) noexcept :
			ptr(ptr), len(len)
		{}

		auto operator[](int i) const noexcept -> T const& { return ptr[i]; }
		auto size() const noexcept -> std::size_t { return len; }

		auto begin() noexcept -> const T* { return ptr; }
		auto end() noexcept -> const T* { return ptr + len; }
	};
}