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
		unsigned int maxFrameMs;
		std::chrono::high_resolution_clock::time_point startTime, endTime;
		float deltaTime;
		unsigned int deltaTimeMs;

		uint32_t wsize;
		uint32_t hsize;

		bool isOpen : 1;
	public:
		const uint32_t& width;
		const uint32_t& height;
	public:
		SH_WINDOW_API Window();
		SH_WINDOW_API ~Window();

		SH_WINDOW_API void Create(const std::string& title, int wsize, int hsize);
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
	};
}