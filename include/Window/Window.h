#pragma once
#pragma warning(disable: 4251)
#include "WindowImpl.h"

#include "Event.h"

#include <memory>
#include <string_view>
#include <queue>
#include <chrono>

namespace sh::window {
	class SH_WINDOW_API Window {
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

		bool isOpen : 1;
	public:
		Window();
		~Window();

		void Create(const std::string& title, int wsize, int hsize);
		bool PollEvent(Event& event);
		
		void Close();

		/// 프레임 제한, 측정 함수
		///
		/// while문 안에 넣을 것
		void ProcessFrame();

		void SetFps(unsigned int fps);
		auto GetDeltaTime() const -> float;
		bool IsOpen() const;

		auto GetNativeHandle() const -> WinHandle;
	};
}