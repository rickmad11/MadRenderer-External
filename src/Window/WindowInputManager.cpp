#include "pch.h"

#include "WindowInputManager.h"
#include "WindowManager.h"

#pragma region KEYBOARD

bool WindowInputManager::IsKeyPressed(unsigned int vKeyCode) noexcept
{
	if (vKeyCode > 255)
		return false;

	return vKeyCodesState[vKeyCode];
}

bool WindowInputManager::IsKeyHeld(unsigned int vKeyCode) noexcept
{
	static unsigned int held_iteration = 0;

	if (vKeyCode > 255)
		return false;

	if(vKeyCodesState[vKeyCode])
	{
		++held_iteration;

		if (held_iteration >= 120)
		{
			held_iteration = 0;
			return true;
		}
	}

	return false;
}

void WindowInputManager::KeyPressed(unsigned int vKeyCode) noexcept
{
	if (vKeyCode > 255)
		return;

	vKeyCodesState[vKeyCode] = true;
}

void WindowInputManager::KeyReleased(unsigned int vKeyCode) noexcept
{
	if (vKeyCode > 255)
		return;

	vKeyCodesState[vKeyCode] = false;
}

bool WindowInputManager::IsKeyReleased(unsigned int vKeyCode) noexcept
{
	if (vKeyCode > 255)
		return false;

	return !vKeyCodesState[vKeyCode];
}

void WindowInputManager::OnCharEvent(wchar_t character) noexcept
{
	qKeysPressed.emplace(character);
	TrimBuffer(qKeysPressed);
}

bool WindowInputManager::IsCharEmpty() noexcept
{
	return qKeysPressed.empty();
}

std::optional<wchar_t> WindowInputManager::ReadChar() noexcept
{
	if(!qKeysPressed.empty())
	{
		wchar_t curr_key = qKeysPressed.front();
		qKeysPressed.pop();
		return curr_key;
	}

	return {};
}

void WindowInputManager::FlushChars() noexcept
{
	qKeysPressed = std::queue<wchar_t>{};
}

void WindowInputManager::FlushKeys() noexcept
{
	vKeyCodesState.reset();
}

void WindowInputManager::FlushKeyboardInputState() noexcept
{
	FlushChars();
	FlushKeys();
}

template<typename T>
void WindowInputManager::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > uMaxBufferSize)
		buffer.pop();
}

#pragma endregion KEYBOARD

#pragma region MOUSE

void WindowInputManager::OnMouseMoveEvent(POINTS point) noexcept
{
	sMousePositionDelta = 
	{
		static_cast<short>(point.x - sCurrentMousePosition.x),
		static_cast<short>(point.y - sCurrentMousePosition.y)
	};

	sCurrentMousePosition = point;
}

POINTS WindowInputManager::GetMousePoints() noexcept
{
	return sCurrentMousePosition;
}

POINTS WindowInputManager::GetMousePointsDelta() noexcept
{
	return sMousePositionDelta;
}

bool WindowInputManager::IsInsideArea(RECT area) noexcept
{
	return (sCurrentMousePosition.x >= area.left && sCurrentMousePosition.x <= area.right && 
			sCurrentMousePosition.y >= area.top && sCurrentMousePosition.y <= area.bottom);
}

void WindowInputManager::ClampMousePosition(WindowManager* p_window_manager) noexcept
{
	RECT client_rect = p_window_manager->GetClientRect();

	sCurrentMousePosition.x = std::clamp(sCurrentMousePosition.x, SHORT(0), static_cast<SHORT>(client_rect.right - client_rect.left));
	sCurrentMousePosition.y = std::clamp(sCurrentMousePosition.y, SHORT(0), static_cast<SHORT>(client_rect.bottom - client_rect.top));
}

void WindowInputManager::EnableMouseCapture() noexcept
{
	bCaptureMouse = true;
}

void WindowInputManager::DisableMouseCapture() noexcept
{
	bCaptureMouse = false;
}

void WindowInputManager::OnMouseWheelEvent(short delta) noexcept
{
	if (delta > 0)
		iMouseWheelDelta += delta / WHEEL_DELTA;
	else
		iMouseWheelDelta -= -(delta / WHEEL_DELTA);
}

int WindowInputManager::GetMouseWheelCount() noexcept
{
	return iMouseWheelDelta;
}

void WindowInputManager::OnMouseLButtonDown() noexcept
{
	bIsLButtonPressed = true;
}

void WindowInputManager::OnMouseRButtonDown() noexcept
{
	bIsRButtonPressed = true;
}

void WindowInputManager::OnMouseLButtonRelease() noexcept
{
	bIsLButtonPressed = false;
}

void WindowInputManager::OnMouseRButtonRelease() noexcept
{
	bIsRButtonPressed = false;
}

bool WindowInputManager::IsLButtonPressed() noexcept
{
	return bIsLButtonPressed;
}

bool WindowInputManager::IsRButtonPressed() noexcept
{
	return bIsRButtonPressed;
}

bool WindowInputManager::IsRButtonReleased() noexcept
{
	return !bIsRButtonPressed;
}

bool WindowInputManager::IsLButtonReleased() noexcept
{
	return !bIsRButtonPressed;
}

#pragma endregion MOUSE