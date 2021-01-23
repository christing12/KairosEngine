#include "krspch.h"
#include "WinWindowSystem.h"

#include "Engine.h"
#include "D3DWindowSurface.h"
#include "Core/Filesystem.h"

#include "Core/Events/KeyEvent.h"
#include "Core/Events/MouseEvent.h"
#include "Core/Events/WindowEvent.h"

#include "Interface/IRenderBackend.h"
extern Kairos::Engine* g_Engine;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace Kairos {

	namespace WinWindowSystemInternal {
		LRESULT MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);
		Ref<IWindowSurface> m_WindowSurface;
		LPCWSTR m_AppName;

		IWindowSystem::EventCallbackFn m_CallbackFunc;

		HINSTANCE m_hInstance;
		HWND m_HWND;
	}

	using namespace WinWindowSystemInternal;

	bool WinWindowSystem::Setup(ISystemConfig* config)
	{
		auto windowConfig = reinterpret_cast<IWindowSystemConfig*>(config);

		m_hInstance = static_cast<HINSTANCE>(windowConfig->AppHook);

		std::string test = g_Engine->GetName();
		KRS_CORE_INFO(test);

		m_AppName = L"Bob";

//		m_AppName = Filesystem::StringToWString(g_Engine->GetName()).c_str();

		m_WindowSurface = CreateRef<D3DWindowSurface>();

		IWindowSurfaceConfig surfaceConfig;
		surfaceConfig.hInstance = m_hInstance;
		surfaceConfig.hWND		= m_HWND;
		surfaceConfig.WndProc	= WindowProc;

		WinWindowSystemInternal::m_WindowSurface->Setup(&surfaceConfig);

		return true;
	}

	bool WinWindowSystem::Init()
	{
		WinWindowSystemInternal::m_WindowSurface->Init();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_HWND;
		::TrackMouseEvent(&tme);
		return true;
	}

	bool WinWindowSystem::Update()
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				return false;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		return true;
	}

	bool WinWindowSystem::Shutdown()
	{
		return true;
	}

	IWindowSurface* WinWindowSystem::GetWindowSurface()
	{
		return m_WindowSurface.get();
	}

	void WinWindowSystem::SetEventCallback(const EventCallbackFn& callback)
	{
		m_CallbackFunc = callback;
	}


	LRESULT WinWindowSystemInternal::MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		//if (ImGui_ImplWin32_WndProcHandler(mHWND, msg, wParam, lParam))
		//{
		//	return true;
		//}
		//const auto& imio = ImGui::GetIO();

		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
		{
			//Uint32 width = LOWORD(lParam);
			//Uint32 height = HIWORD(lParam);
			//WindowResizeEvent event(width, height);

			//mCallbackFunc(event);
			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			int key = (int)wParam;
			//KRS_CORE_INFO(key);
			KeyPressedEvent event((KeyCode)key, 0);
			m_CallbackFunc(event);
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			KeyReleasedEvent event((KeyCode)(int)wParam);
			m_CallbackFunc(event);
			break;
		}

		case WM_MOUSEMOVE:
		{
			auto [width, height] = std::pair<Uint32, Uint32>(1024, 1024);   //g_Engine->GetRenderBackend()->GetScreenResolution();
			const POINTS pt = MAKEPOINTS(lParam);
			if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
				MouseMovedEvent e(static_cast<float>(pt.x), static_cast<float>(pt.y));
				m_CallbackFunc(e);
			}
			else {
				if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
					MouseMovedEvent e(static_cast<float>(pt.x), static_cast<float>(pt.y));
					m_CallbackFunc(e);
				}
			}
		}

		case WM_MBUTTONDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			const POINTS pt = MAKEPOINTS(lParam);

			MouseButtonPressedEvent event((MouseCode)(int)wParam);
			m_CallbackFunc(event);
			break;
		}

		case WM_MBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			MouseButtonReleasedEvent event(Mouse::MiddleButton);
			m_CallbackFunc(event);
			break;
		}
		case WM_LBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			MouseButtonReleasedEvent event(Mouse::LeftButton);
			m_CallbackFunc(event);
			break;
		}
		case WM_RBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			MouseButtonReleasedEvent event(Mouse::RightButton);
			m_CallbackFunc(event);
			break;
		}

		case WM_MOUSELEAVE:
		{
			std::cout << " EWIFHPW" << std::endl;
			break;
		}

		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}





	using namespace WinWindowSystemInternal;

	HWND WinWindowSystem::GetHWND()
	{
		return m_HWND;
	}

	HINSTANCE WinWindowSystem::GetHINSTANCE()
	{
		return m_hInstance;
	}

	bool WinWindowSystem::SetHWND(HWND hwnd)
	{
		m_HWND = hwnd;
		return true;
	}

	LPCWSTR WinWindowSystem::GetAppName()
	{
		return m_AppName;
	}



}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// For to eliminate fake OpenGL window handle event
	if (hwnd != Kairos::WinWindowSystemInternal::m_HWND)
	{
		return Kairos::WinWindowSystemInternal::MessageHandler(hwnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_SIZE:
	{
		//if (lParam && g_Engine->GetStatus() == ObjectStatus::Activated)
		//{
		//	auto l_width = lParam & 0xffff;
		//	auto l_height = (lParam & 0xffff0000) >> 16;

		//	TVec2<uint32_t> l_newResolution = TVec2<uint32_t>((uint32_t)l_width, (uint32_t)l_height);
		//	g_Engine->getRenderingFrontend()->setScreenResolution(l_newResolution);
		//	g_Engine->getRenderingServer()->Resize();
		//}
	}
	default:
	{
		return Kairos::WinWindowSystemInternal::MessageHandler(hwnd, uMsg, wParam, lParam);
	}
	}
}