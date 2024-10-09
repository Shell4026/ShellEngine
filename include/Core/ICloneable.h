#pragma once

#include <memory>
namespace sh::core
{
	/// @brief Clone()을 통해 자기 자신의 복사본을 만들 수 있는 인터페이스
	/// @tparam T 타입
	template<typename T>
	class ICloneable
	{
	public:
		virtual ~ICloneable() = default;

		/// @brief 자기 자신의 복사본을 반환하는 함수
		/// @return 복사본의 스마트 포인터
		virtual auto Clone() const -> std::unique_ptr<T> = 0;
	};
}//namespace