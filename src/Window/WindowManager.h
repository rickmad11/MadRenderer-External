#pragma once

#include "WindowInputManager.h"
#include "WindowMessageManager.h"

class WindowManager final : public WindowMessageManager, public WindowInputManager
{
public:
	WindowManager(WindowManager const&) = delete;
	WindowManager& operator=(WindowManager const&) = delete;

	WindowManager(WindowManager&&) = delete;
	WindowManager& operator=(WindowManager&&) = delete;

private:
	WindowManager() = default;
	virtual ~WindowManager() = default;

public:
	static WindowManager& Get();
	bool InitializeWindow(std::wstring_view window_name, int windowWidth = 0, int windowHeight = 0, std::wstring_view class_name = L"DirectX11 Testing Class", HMODULE owner_module = nullptr);
	void Exit() noexcept;

public:
	HWND GetWindowHandle() const;
	RECT GetClientRect() const;
	int GetWindowWidth() const noexcept;
	int GetWindowHeight() const noexcept;
	void SetWindowSize(UINT new_width, UINT new_height) noexcept;
	void ChangeToFullscreenMode() const noexcept;
	void ChangeWindowSize(UINT new_width, UINT new_height) noexcept;
	void FollowWindow(HWND window) noexcept;
	bool HasWindowMoved(HWND window, RECT& r_window_rect) noexcept;
	void HideFromCapture(bool hide) const noexcept;
	void UpdateWindowRectData() noexcept;

public:
	using WindowMessageManager::WindowMessageLoop;

public:
	static inline std::atomic_bool IsWindowValid = false;

private:
	bool m_is_initialized = false;

private:
	RECT m_client_rect{};
	RECT m_window_rect{};
	bool window_rect_updated = false;
	HMODULE m_current_module = nullptr;
	HWND m_window_handle = nullptr;
	std::wstring m_class_name {};

private:
	int m_window_width = 0;
	int m_window_height = 0;
	std::wstring m_window_name{};
};
