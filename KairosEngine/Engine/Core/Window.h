#pragma once

#include "EngineCore.h"
#include "BaseTypes.h"
#include "Events/Event.h"

#include <functional>
#include <optional>
#include <Windows.h>

KRS_BEGIN_NAMESPACE(Kairos)



// abstract class / interface so its platform agnostic (do i really need this?)
class Window {
public:
	Window() = default;
	virtual ~Window() = default;

	void Init(Uint32 width, Uint32 height);
	void Shutdown();
	std::optional<int> ProcessEvents();


	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProcThunk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK InternalWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	using EventCallbackFn = std::function<void(Event&)>;
	HWND GetNativeWindow() { return mHWND; }
	void SetEventCallback(const EventCallbackFn& callback) { mCallbackFunc = callback; }

private:
	Uint32 mWidth = 1280;
	Uint32 mHeight = 1024;
	HWND mHWND;
	HINSTANCE mInstance;

	EventCallbackFn mCallbackFunc;
private:
	void RegisterWindowClass(HINSTANCE instance, const wchar_t* windowClassName);
	void CreateNewWindow(const wchar_t* windowClassName, HINSTANCE instance, const wchar_t* title,
		Uint32 width, Uint32 height);


};


KRS_END_NAMESPACE