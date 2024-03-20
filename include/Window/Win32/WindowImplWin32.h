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
	public:
		WindowImplWin32();
		~WindowImplWin32();

		auto Create(const std::string& title, int wsize, int hsize) -> WinHandle override;
		void Close() override;
		void ProcessEvent() override;
	};
}