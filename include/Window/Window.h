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

namespace sh::window {
	class Window {
	private:
		std::unique_ptr<WindowImpl> winImpl;
		WinHandle handle;

		std::string title;

		std::queue<Event> events;

		unsigned int fps;
		float maxFrameMs;
		std::chrono::high_resolution_clock::time_point startTime, endTime;
		float deltaTime;

		uint32_t wsize;
		uint32_t hsize;

		bool isOpen : 1;
	public:
		const uint32_t& width;
		const uint32_t& height;

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

		/// 프레임 제한, 측정 함수
		///
		/// while문 안에 넣을 것
		SH_WINDOW_API void ProcessFrame();

		SH_WINDOW_API void SetFps(unsigned int fps);
		SH_WINDOW_API auto GetDeltaTime() const -> float;
		SH_WINDOW_API bool IsOpen() const;

		SH_WINDOW_API auto GetNativeHandle() const -> WinHandle;

		SH_WINDOW_API void SetTitle(std::string_view title);

		SH_WINDOW_API auto GetWidth() const -> uint32_t;
		SH_WINDOW_API auto GetHeight() const -> uint32_t;
	};
}