#include "krspch.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "Events/KeyEvent.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Standard WndProc -> sends to custom instance's message handler if no system end events
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	//	return 0;

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}


namespace Kairos {
	Window::Window(const WindowProps& props)
		: width(props.Width)
		, height(props.Height)
		, mTitle(props.Title)
	{
		Init();
	}

	void Window::Init() {
		mInstance = GetModuleHandle(NULL);
		RegisterWindowClass(mInstance, L"Mesh");

		mHWND = CreateNewWindow(L"Mesh", mInstance, L"Mesh", width, height);
		bool test = ShowWindow(mHWND, SW_SHOW);
	}

	void Window::Shutdown()
	{
		ShowCursor(true);
		DestroyWindow(mHWND);
		mHWND = nullptr;

		UnregisterClass("Mesh", mInstance);
	}

	void Window::RegisterWindowClass(HINSTANCE instance, const wchar_t* windowClassName)
	{
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW; // redrawn if width / height changes / movement
		wcex.lpfnWndProc = WndProc; // pointer to windows procedure that handles window messages
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = instance; // contains windows procedure
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hIcon = LoadIcon(instance, NULL);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = windowClassName;
		wcex.hIconSm = LoadIcon(instance, NULL);
		static ATOM atom = ::RegisterClassExW(&wcex);

		KRS_ASSERT(atom > 0, "Issue with registering window class");
	}

	HWND Window::CreateNewWindow(const wchar_t* windowClassName, HINSTANCE instance, const wchar_t* title, Uint32 width, Uint32 height)
	{
		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		RECT rect = { 0 , 0, static_cast<LONG>(width), static_cast<LONG>(height) };
		::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		int windowWidth = rect.right - rect.left;
		int windowHeight = rect.bottom - rect.top;

		int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

		HWND hWnd = ::CreateWindowExW(NULL, windowClassName, title, WS_OVERLAPPEDWINDOW,
			windowX, windowY, windowWidth, windowHeight, NULL, NULL, instance, nullptr);

		KRS_ASSERT(hWnd != nullptr, "Error with creating window instance");
		return hWnd;
	}

	std::optional<int> Window::ProcessEvents()
	{
		MSG msg;
		// Empty out message queue
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			switch (msg.message)
			{
			case WM_QUIT:
				return (int)msg.wParam;
			case WM_KEYDOWN:
			{
				int key = (int)msg.wParam;
				KeyPressedEvent event((KeyCode)key, 0);
				mCallbackFunc(event);
				break;
			}
			case WM_KEYUP:
			{
				KeyReleasedEvent event((KeyCode)(int)msg.wParam);
				mCallbackFunc(event);
				break;
			}
			case WM_MBUTTONDOWN:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			{
				MouseButtonPressedEvent event((MouseCode)(int)msg.wParam);
				mCallbackFunc(event);
				break;
			}

			case WM_MBUTTONUP:
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			{
				MouseButtonReleasedEvent event((MouseCode)(int)msg.wParam);
				mCallbackFunc(event);
				break;
			}

			}
			// will generate wchar* message for keydown and puts it in message queue
			TranslateMessage(&msg);
			// passes message back to Win32 side -> passes it to WndProcedure callback
			DispatchMessage(&msg);
		}
		return {};
	}
}