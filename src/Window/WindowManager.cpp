#include "pch.h"

#include "WindowManager.h"

WindowManager& WindowManager::Get()
{
	static WindowManager s_WindowManager;
	return s_WindowManager;
}

bool WindowManager::InitializeWindow(std::wstring_view window_name, int windowWidth, int windowHeight, std::wstring_view class_name, HMODULE owner_module)
{
	if(!m_is_initialized)
	{
		m_is_initialized = true;
		m_window_name	 = window_name;

		if (!windowHeight || !windowWidth)
		{
			m_window_width = GetSystemMetrics(SM_CXSCREEN) / 2;
			m_window_height = GetSystemMetrics(SM_CYSCREEN) / 2;
		}
		else
		{
			m_window_width = windowWidth;
			m_window_height = windowHeight;
		}

		if (owner_module == nullptr)
			m_current_module = GetModuleHandleW(nullptr);
		else
			m_current_module = owner_module;


		WNDCLASSEXW wndclassexw
		{
			.cbSize			= sizeof(WNDCLASSEXW),
			.style			= 0,
			.lpfnWndProc	= WindowProcWThunk,
			.cbClsExtra		= 0,
			.cbWndExtra		= 0,
			.hInstance		= m_current_module,
			.hIcon			= nullptr,
			.hbrBackground  = nullptr,
			.lpszMenuName	= nullptr,
			.lpszClassName  = class_name.data(),
			.hIconSm		= nullptr
		};

		ATOM class_atom = RegisterClassExW(&wndclassexw);

		DWORD dwExStyleFlags = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT;
		DWORD dwStyleFlags = WS_POPUP;

		RECT window_rect
		{
			.right = m_window_width,
			.bottom = m_window_height
		};

		AdjustWindowRect(&window_rect, dwStyleFlags, false);

		HWND window_handle = CreateWindowExW(dwExStyleFlags, MAKEINTATOM(class_atom), window_name.data(),
			dwStyleFlags, CW_USEDEFAULT, CW_USEDEFAULT,
			window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, nullptr,
			nullptr, m_current_module, this);

		if (window_handle == nullptr)
			return false;

		m_window_handle = window_handle;

		DWM_BLURBEHIND bb = { DWM_BB_ENABLE | DWM_BB_BLURREGION, true, CreateRectRgn(0, 0, -1, -1), true };
		SetLayeredWindowAttributes(m_window_handle, NULL, NULL, NULL);
		DwmEnableBlurBehindWindow(m_window_handle, &bb);

		ShowWindow(window_handle, SW_SHOW);
		UpdateWindow(window_handle);

		::GetClientRect(window_handle, &m_client_rect);

		IsWindowValid = true;

		return true;
	}

	return false;
}

HWND WindowManager::GetWindowHandle() const
{
	return IsWindowValid ? m_window_handle : nullptr;
}

RECT WindowManager::GetClientRect() const
{
	return m_client_rect;
}

int WindowManager::GetWindowWidth() const noexcept
{
	return m_window_width;
}

int WindowManager::GetWindowHeight() const noexcept
{
	return m_window_height;
}

void WindowManager::SetWindowSize(UINT new_width, UINT new_height) noexcept
{
	m_window_height = new_height;
	m_window_width  = new_width;
}

void WindowManager::ChangeToFullscreenMode() const noexcept
{
	SendMessageW(m_window_handle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
}

void WindowManager::ChangeWindowSize(UINT new_width, UINT new_height) noexcept
{
	SetWindowSize(new_width, new_height);

	RECT window_rect {};
	GetWindowRect(m_window_handle, &window_rect);

	LPARAM lparam = ((0xFFFF & new_width) | new_height << 16);
	SendMessageW(m_window_handle, WM_SIZE, 0, lparam);

	MoveWindow(m_window_handle, window_rect.left, window_rect.top, new_width, new_height, false);
}

void WindowManager::FollowWindow(HWND window) const noexcept
{
	if(window)
	{
		RECT window_rect{};
		if (HasWindowMoved(window, window_rect))
		{
			UINT new_width = window_rect.right - window_rect.left;
			UINT new_height = window_rect.bottom - window_rect.top;

			if (new_width != static_cast<UINT>(m_window_width) || new_height != static_cast<UINT>(m_window_height))
			{
				LPARAM lparam = ((0xFFFF & new_width) | new_height << 16);
				SendMessageW(m_window_handle, WM_SIZE, 0, lparam);
			}

			MoveWindow(m_window_handle, window_rect.left, window_rect.top, new_width, new_height, false);
		}
	}
}

bool WindowManager::HasWindowMoved(HWND window, RECT& r_window_rect) const noexcept
{
	if(window)
	{
		GetWindowRect(window, &r_window_rect);

		return r_window_rect != m_window_rect;
	}

	return false;
}

void WindowManager::HideFromCapture(bool hide) const noexcept
{
	SetWindowDisplayAffinity(m_window_handle, hide ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
}

WindowManager::~WindowManager()
{
	(void)UnregisterClassW(m_class_name.data(), m_current_module);
	(void)DestroyWindow(m_window_handle);
}