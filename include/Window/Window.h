#pragma once
#pragma warning(disable: 4251)
#include "WindowImpl.h"

#include "Event.h"

#include <memory>
#include <string_view>
#include <queue>
#include <chrono>
#include <string_view>
#include <stdint.h>

namespace sh::window 
{
	class Window 
	{
	public:
		using StyleFlag = uint32_t;
		enum Style
		{
			Default = 0b0000,
			Resize = 0b0001,
			Full = 0b0010
		};
	public:
		SH_WINDOW_API Window();
		SH_WINDOW_API ~Window();

		SH_WINDOW_API void Create(const std::string& title, int wsize, int hsize, StyleFlag style = Style::Default);
		SH_WINDOW_API bool PollEvent(Event& event);
		
		SH_WINDOW_API void Close();

		/// @brief 프레임 제한, 측정 함수
		/// @brief while문 안에 넣을 것
		SH_WINDOW_API void ProcessFrame();

		SH_WINDOW_API void SetFps(unsigned int fps);
		SH_WINDOW_API auto GetDeltaTime() const -> float;
		SH_WINDOW_API bool IsOpen() const;

		SH_WINDOW_API auto GetNativeHandle() const -> WinHandle;

		SH_WINDOW_API void SetTitle(std::string_view title);

		SH_WINDOW_API void SetSize(int width, int height);

		SH_WINDOW_API auto GetWidth() const -> uint32_t;
		SH_WINDOW_API auto GetHeight() const -> uint32_t;

		/// @brief 프레임을 맞출 때 시스템 타이머를 사용한다.
		/// @brief 주의) 백그라운드 상태에서 프레임이 느려질 수 있음
		/// @param bUse true면 시스템 타이머를, false면 바쁜 대기를 사용
		SH_WINDOW_API void UseSystemTimer(bool bUse);
		SH_WINDOW_API auto IsUseingSystemTimer() const -> bool;
	public:
		const uint32_t& width;
		const uint32_t& height;
	private:
		std::unique_ptr<WindowImpl> winImpl;
		WinHandle handle;

		std::string title;

		std::queue<Event> events;

		unsigned int fps;
		float deltaTime;

		uint32_t wsize;
		uint32_t hsize;

		bool isOpen;
		bool bUsingSysTimer = true;
	};
}//namespace