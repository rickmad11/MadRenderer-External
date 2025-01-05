#include "pch.h"

#include "WindowMessageManager.h"
#include "WindowManager.h"
#include "../Graphics/DX11.h"

LRESULT __stdcall WindowMessageManager::WindowProcWThunk(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	if(Msg == WM_CREATE)
	{
		CREATESTRUCTA* const p_createstructa = reinterpret_cast<CREATESTRUCTA*>(lParam);
		WindowManager* const p_window_manager = static_cast<WindowManager*>(p_createstructa->lpCreateParams);
		(void)SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcWRedirect));
		(void)SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(p_window_manager));

		return p_window_manager->WindowProcWR(hWnd, Msg, wParam, lParam);
	}

	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

LRESULT __stdcall WindowMessageManager::WindowProcWRedirect(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static LONG_PTR p_class_instance = GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	return reinterpret_cast<WindowMessageManager*>(p_class_instance)->WindowProcWR(hWnd, Msg, wParam, lParam);
}

LRESULT WindowMessageManager::WindowProcWR(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static bool isMouseCaptured = false;

	switch (Msg)
	{
		case WM_CLOSE:
			[[fallthrough]];
		case WM_DESTROY:
			PostQuitMessage(WM_QUIT);
			return 0;

		case WM_KEYDOWN:
			[[fallthrough]];
		case WM_SYSKEYDOWN:
			if( !(lParam & (1 << 30)) ) //Autorepeat
				WindowInputManager::KeyPressed(static_cast<unsigned char>(wParam));
			break;

		case WM_KEYUP:
			[[fallthrough]];
		case WM_SYSKEYUP:
			WindowInputManager::KeyReleased(static_cast<unsigned char>(wParam));
			break;

		case WM_CHAR:
			WindowInputManager::OnCharEvent(static_cast<wchar_t>(wParam));
			break;

		case WM_MOUSEMOVE:
			WindowInputManager::OnMouseMoveEvent(MAKEPOINTS(lParam));

			if (WindowInputManager::bCaptureMouse)
				WindowInputManager::ClampMousePosition(reinterpret_cast<WindowManager*>(const_cast<WindowMessageManager*>(this)));
			break;

		case WM_RBUTTONDOWN:
			if (WindowInputManager::bCaptureMouse && !isMouseCaptured)
			{
				SetCapture(hWnd);
				isMouseCaptured = true;
			}
			WindowInputManager::OnMouseRButtonDown();
			break;

		case WM_LBUTTONDOWN:
			if(WindowInputManager::bCaptureMouse && !isMouseCaptured)
			{
				SetCapture(hWnd);
				isMouseCaptured = true;
			}
			WindowManager::OnMouseLButtonDown();
			break;

		case WM_RBUTTONUP:
			if (WindowInputManager::bCaptureMouse && isMouseCaptured)
			{
				ReleaseCapture();
				isMouseCaptured = false;
			}
			WindowManager::OnMouseRButtonRelease();
			break;

		case WM_LBUTTONUP:
			if (WindowInputManager::bCaptureMouse && isMouseCaptured)
			{
				ReleaseCapture();
				isMouseCaptured = false;
			}
			WindowManager::OnMouseLButtonRelease();
			break;
			
		case WM_MOUSEWHEEL:
			WindowInputManager::OnMouseWheelEvent(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_KILLFOCUS:
			WindowInputManager::FlushKeyboardInputState();
			break;

		case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);

				reinterpret_cast<WindowManager*>(this)->SetWindowSize(width, height);
				DX11::OnWindowResize(width, height);
				break;
			}

		default:
			break;
	}

	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

void WindowMessageManager::WindowMessageLoop(void(__stdcall *pCallback)(void)) const
{
	if (pCallback == nullptr)
		return;

	MSG window_message{};

	while (true)
	{
		if (PeekMessageW(&window_message, nullptr, NULL, NULL, PM_REMOVE))
		{
			if (window_message.message == WM_QUIT)
			{
				WindowManager::IsWindowValid = false;
				return;
			}

			TranslateMessage(&window_message);
			DispatchMessageW(&window_message);
		}

		pCallback();
	}
}
