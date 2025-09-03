#pragma once
#include "Export.h"
#include "../WindowImpl.h"

#include <Windows.h>
namespace sh::window 
{
	class WindowImplWin32 : public WindowImpl 
	{
	public:
		SH_WINDOW_API WindowImplWin32();
		SH_WINDOW_API ~WindowImplWin32();

		SH_WINDOW_API auto Create(const std::string& title, int wsize, int hsize, uint32_t style) -> WinHandle override;
		SH_WINDOW_API void Close() override;
		SH_WINDOW_API void ProcessEvent() override;
		SH_WINDOW_API void SetTitle(std::string_view title) override;
		SH_WINDOW_API auto GetWidth() const -> uint32_t override;
		SH_WINDOW_API auto GetHeight() const->uint32_t override;
		SH_WINDOW_API void StopTimer(uint32_t ms) override;
		SH_WINDOW_API void Resize(int width, int height) override;
	private:
		static LRESULT CALLBACK EventHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void RegisterWindow();
		void ProcessEvents(UINT msg, WPARAM wParam, LPARAM lParam);
		/*윈도우 이벤트의 wParam값을 Event::KeyType으로 변환하는 함수*/
		auto ConvertKeycode(WPARAM wParam) -> Event::KeyType;
	private:
		const wchar_t* className;

		HWND window;
	};
}