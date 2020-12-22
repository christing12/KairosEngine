#pragma once

#include "EngineCore.h"
#include "BaseTypes.h"
#include "Events/Event.h"

#include <functional>
#include <optional>
#include <Windows.h>

KRS_BEGIN_NAMESPACE(Kairos)


struct WindowProps {
	std::string Title;
	unsigned int Width;
	unsigned int Height;

	WindowProps(const std::string& title = "Kairos Engine",
		unsigned int width = 1280,
		unsigned int height = 1024)
		: Title(title), Width(width), Height(height)
	{}
};

// abstract class / interface so its platform agnostic (do i really need this?)
class Window {
public:
	using EventCallbackFn = std::function<void(Event&)>;
	Window() = default;
	Window(const WindowProps& props);
	virtual ~Window() = default;

	void Init();
	void Shutdown();
	std::optional<int> ProcessEvents();

	void SetEventCallback(const EventCallbackFn& callback) { mCallbackFunc = callback; }

	HWND GetNativeWindow() { return mHWND; }
private:
	Uint32 width = 1280;
	Uint32 height = 1024;
	std::string mTitle;
	EventCallbackFn mCallbackFunc;
	HWND mHWND;
	HINSTANCE mInstance;

private:
	void RegisterWindowClass(HINSTANCE instance, const wchar_t* windowClassName);
	HWND CreateNewWindow(const wchar_t* windowClassName, HINSTANCE instance, const wchar_t* title,
		Uint32 width, Uint32 height);
};


KRS_END_NAMESPACE