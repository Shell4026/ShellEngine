#pragma once

namespace sh::core 
{
	/// @brief 복사 불가 인터페이스
	class INonCopyable 
	{
	public:
		INonCopyable() = default;
		virtual ~INonCopyable() = default;

		INonCopyable(const INonCopyable& other) = delete;
		void operator=(const INonCopyable& other) = delete;
	};
}