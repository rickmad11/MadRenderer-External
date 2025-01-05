#pragma once

class WindowInputManager
{
public:
	WindowInputManager(WindowInputManager const&) = delete;
	WindowInputManager& operator=(WindowInputManager const&) = delete;

	WindowInputManager(WindowInputManager&&) = delete;
	WindowInputManager& operator=(WindowInputManager&&) = delete;

protected:
	WindowInputManager() = default;
	virtual ~WindowInputManager() = default;

	//--------------------Keyboard Manager--------------------
public:
	static bool IsKeyPressed(unsigned int vKeyCode) noexcept;
	static bool IsKeyHeld(unsigned int vKeyCode) noexcept;
	static void KeyPressed(unsigned int vKeyCode) noexcept;
	static void KeyReleased(unsigned int vKeyCode) noexcept;
	static bool IsKeyReleased(unsigned int vKeyCode) noexcept;

	static void OnCharEvent(wchar_t character) noexcept;
	static bool IsCharEmpty() noexcept;
	static std::optional<wchar_t> ReadChar() noexcept;

public:
	static void FlushChars() noexcept;
	static void FlushKeys() noexcept;
	static void FlushKeyboardInputState() noexcept;

private:
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

private:
	static constexpr unsigned int uMaxBufferSize = 16u;

	inline static std::bitset<255> vKeyCodesState {};
	inline static std::queue<wchar_t> qKeysPressed {};
	//--------------------Keyboard Manager--------------------

	//--------------------Mouse Manager--------------------
public:
	static void OnMouseMoveEvent(POINTS point) noexcept;
	static POINTS GetMousePoints() noexcept;
	static POINTS GetMousePointsDelta() noexcept;

public:
	static bool IsInsideArea(RECT area) noexcept;
	static void ClampMousePosition(class WindowManager* p_window_manager) noexcept;
	static void EnableMouseCapture() noexcept;
	static void DisableMouseCapture() noexcept;
	static void OnMouseWheelEvent(short delta) noexcept;
	static int GetMouseWheelCount() noexcept;
	static void OnMouseLButtonDown() noexcept;
	static void OnMouseRButtonDown() noexcept;
	static void OnMouseLButtonRelease() noexcept;
	static void OnMouseRButtonRelease() noexcept;
	static bool IsLButtonPressed() noexcept;
	static bool IsRButtonPressed() noexcept;
	static bool IsLButtonReleased() noexcept;
	static bool IsRButtonReleased() noexcept;

public:
	static inline bool bCaptureMouse = false;

private:
	static inline bool bIsLButtonPressed = false;
	static inline bool bIsRButtonPressed = false;

private:
	static inline POINTS sCurrentMousePosition{};
	static inline POINTS sMousePositionDelta{};
	static inline int iMouseWheelDelta = 0;
	//--------------------Mouse Manager--------------------
};
