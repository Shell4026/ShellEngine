#pragma once

#include "../WindowImpl.h"

#include <Windows.h>
namespace sh {
	class WindowImplWin32 : public WindowImpl {
	private:
		const wchar_t* className;

		HWND window;
	private:
		static LRESULT CALLBACK EventHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void RegisterWindow();
		void ProcessEvents(UINT msg, WPARAM wParam, LPARAM lParam);
		/*윈도우 이벤트의 wParam값을 Event::KeyType으로 변환하는 함수*/
		auto ConvertKeycode(WPARAM wParam) -> Event::KeyType;
	public:
		WindowImplWin32();
		~WindowImplWin32();

		auto Create(const std::string& title, int wsize, int hsize) -> WinHandle override;
		void Close() override;
		void ProcessEvent() override;
	};
}