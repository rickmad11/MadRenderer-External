#pragma once

class WindowMessageManager
{
public:
	WindowMessageManager(WindowMessageManager const&) = delete;
	WindowMessageManager& operator=(WindowMessageManager const&) = delete;

	WindowMessageManager(WindowMessageManager&&) = delete;
	WindowMessageManager& operator=(WindowMessageManager&&) = delete;

protected:
	WindowMessageManager() = default;
	virtual ~WindowMessageManager() = default;

protected:
	static LRESULT WINAPI WindowProcWThunk(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	static LRESULT WINAPI WindowProcWRedirect(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProcWR(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

protected:
	void WindowMessageLoop(void (__stdcall *pCallback)(void) = nullptr) const;
};

