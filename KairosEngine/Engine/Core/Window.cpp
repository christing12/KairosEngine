#include "krspch.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "Events/KeyEvent.h"
#include "Events/WindowEvent.h"
#include "Events/MouseEvent.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);




namespace Kairos {
	void Window::Init(Uint32 _width, Uint32 _height) {
		mWidth = _width;
		mHeight = _height;
		mInstance = GetModuleHandle(NULL);

		{
			WNDCLASSEXW wcex = { 0 };
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW; // redrawn if width / height changes / movement
			wcex.lpfnWndProc = StaticWndProc; // pointer to windows procedure that handles window messages
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = mInstance; // contains windows procedure
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hIcon = LoadIcon(mInstance, NULL);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = L"Mesh";
			wcex.hIconSm = LoadIcon(mInstance, NULL);
			static ATOM atom = ::RegisterClassExW(&wcex);

			KRS_ASSERT(atom > 0, "Issue with registering window class");
		}

		{

			int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

			RECT rect = { 0 , 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
			::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

			int windowWidth = rect.right - rect.left;
			int windowHeight = rect.bottom - rect.top;

			int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
			int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

			mHWND = ::CreateWindowEx(NULL, L"Mesh", L"Mesh", WS_OVERLAPPEDWINDOW,
				windowX, windowY, windowWidth, windowHeight, NULL, NULL, mInstance, this);

			KRS_ASSERT(mHWND != nullptr, "Error with creating window instance");
		}

		ShowWindow(mHWND, SW_SHOW);
	}

	void Window::Shutdown()
	{
		ShowCursor(true);
		DestroyWindow(mHWND);
		mHWND = nullptr;

		UnregisterClass(L"Mesh", mInstance);
	}

	void Window::RegisterWindowClass(HINSTANCE instance, const wchar_t* windowClassName)
	{
		WNDCLASSEXW wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW; // redrawn if width / height changes / movement
		wcex.lpfnWndProc = StaticWndProc; // pointer to windows procedure that handles window messages
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

	void Window::CreateNewWindow(const wchar_t* windowClassName, HINSTANCE instance, const wchar_t* title, Uint32 width, Uint32 height)
	{
		int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

		RECT rect = { 0 , 0, static_cast<LONG>(width), static_cast<LONG>(height) };
		::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		int windowWidth = rect.right - rect.left;
		int windowHeight = rect.bottom - rect.top;

		int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
		int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

		mHWND = ::CreateWindowEx(NULL, windowClassName, title, WS_OVERLAPPEDWINDOW,
			windowX, windowY, windowWidth, windowHeight, NULL, NULL, instance, this);

		KRS_ASSERT(mHWND != nullptr, "Error with creating window instance");
	}

	LRESULT Window::InternalWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//if (ImGui_ImplWin32_WndProcHandler(mHWND, msg, wParam, lParam))
		//{
		//	return true;
		//}

		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
		{
			int key = (int)wParam;
			KeyPressedEvent event((KeyCode)key, 0);
			mCallbackFunc(event);
			break;
		}
		case WM_KEYUP:
		{
			KeyReleasedEvent event((KeyCode)(int)wParam);
			mCallbackFunc(event);
			break;
		}
		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			MouseButtonPressedEvent event((MouseCode)(int)wParam);
			mCallbackFunc(event);
			break;
		}

		case WM_MBUTTONUP:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		{
			MouseButtonReleasedEvent event((MouseCode)(int)wParam);
			mCallbackFunc(event);
			break;
		}

		}
		return DefWindowProc(mHWND, msg, wParam, lParam);
	}

	LRESULT Window::WndProcThunk(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		Window* pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return pWnd->InternalWndProc(message, wParam, lParam);
	}

	LRESULT Window::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_NCCREATE) {
			Window* pWnd = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

			pWnd->mHWND = hWnd;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::WndProcThunk));
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	std::optional<int> Window::ProcessEvents()
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				return (int)msg.wParam;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		return {};
	}
}