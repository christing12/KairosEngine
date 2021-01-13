#include "krspch.h"
#include "D3DWindowSurface.h"
#include "WinWindowSystem.h"

#include "Interface/IEngine.h"

extern Kairos::IEngine* g_Engine;

namespace Kairos {
	namespace D3DWindowSurfaceInternal {
		bool Setup(ISystemConfig* config);
		bool Init();
		bool Update();
		bool Shutdown();

		HWND m_HWND;
		HINSTANCE m_Instance;
	}

	bool D3DWindowSurfaceInternal::Setup(ISystemConfig* config) {
		auto surfaceConfig = reinterpret_cast<IWindowSurfaceConfig*>(config);

		std::string appName = g_Engine->GetName();
		HINSTANCE instance = reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetHINSTANCE();

		{
			WNDCLASSEXW wcex = { 0 };
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW; // redrawn if width / height changes / movement
			wcex.lpfnWndProc = (WNDPROC) surfaceConfig->WndProc; // pointer to windows procedure that handles window messages
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = instance; // contains windows procedure
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hIcon = LoadIcon(instance, NULL);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetAppName();
			wcex.hIconSm = LoadIcon(instance, NULL);
			static ATOM atom = ::RegisterClassExW(&wcex);

			KRS_ASSERT(atom > 0, "Issue with registering window class");
		}



		{
			int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

			//auto [width, height] = g_Engine->GetRenderBackend()->GetScreenResolution();
			RECT rect = { 0 , 0, static_cast<LONG>(1024), static_cast<LONG>(1024) };
			::AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

			int windowWidth = rect.right - rect.left;
			int windowHeight = rect.bottom - rect.top;

			int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
			int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

			HWND hwnd = ::CreateWindowEx(
				NULL, 
				reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetAppName(),
				reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetAppName(),
				WS_OVERLAPPEDWINDOW,
				windowX, windowY, windowWidth, windowHeight, NULL, NULL, instance, NULL);

			KRS_ASSERT(hwnd != nullptr, "Error with creating window instance");
			reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->SetHWND(hwnd);
		}

		KRS_CORE_INFO("DirectX Surface setup!");
		return true;
	}

	bool D3DWindowSurfaceInternal::Init()
	{
		HWND hwnd = reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetHWND();
		ShowWindow(hwnd, true);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return true;
	}

	bool D3DWindowSurfaceInternal::Update()
	{
		return true;
	}

	bool D3DWindowSurfaceInternal::Shutdown()
	{
		KRS_CORE_INFO("DXSurface shut down");
		return false;
	}


	bool D3DWindowSurface::Setup(ISystemConfig* config)
	{
		return D3DWindowSurfaceInternal::Setup(config);
	}

	bool D3DWindowSurface::Init()
	{
		return D3DWindowSurfaceInternal::Init();
	}

	bool D3DWindowSurface::Update()
	{
		return D3DWindowSurfaceInternal::Update();
	}

	bool D3DWindowSurface::Shutdown()
	{
		return D3DWindowSurfaceInternal::Shutdown();
	}

}

