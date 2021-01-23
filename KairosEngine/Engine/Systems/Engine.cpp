#include "krspch.h"
#include "Engine.h"

#include "Interface/IEventSystem.h"
#include "WinWindowSystem.h"

#include "RenderBackend/DX12RenderBackend.h"
#include "WinWindowSystem.h"
#include "InputSystem.h"
#include "GUISystem.h"


Kairos::Engine* g_Engine;


#define GetSystemDef( system ) \
I##system * Engine::Get##system() \
{ \
	return g_##system.get(); \
} \


namespace Kairos {

	bool Engine::Setup(void* appHook, void* extraHook, char* cmdLine) {
		g_Engine = this;
		// create systems then setup them up
		g_RenderBackend		= CreateScope<DX12RenderBackend>();
		g_WindowSystem		= CreateScope<WinWindowSystem>();
		g_InputSystem		= CreateScope<InputSystem>();
		g_GUISystem			= CreateScope<GUISystem>();

		IWindowSystemConfig windowConfig;
		windowConfig.AppHook = appHook;
		windowConfig.ExtraHook = extraHook;
		if (!g_WindowSystem->Setup(&windowConfig)) {
			KRS_CORE_ERROR("Error With Registering Window");
			return false;
		}

		IRenderBackendConfig renderConfig;
		renderConfig.NumFrames = 2;
		if (!g_RenderBackend->Setup(&renderConfig)) {
			KRS_CORE_ERROR("Error with creating render backend");
			return false;
		}

		if (!g_InputSystem->Setup(nullptr)) {
			KRS_CORE_ERROR("Error with creating Input System");
			return false;
		}

		if (!g_GUISystem->Setup(nullptr)) {
			KRS_CORE_ERROR("Error with Creating gui system");
			return false;
		}



		return true;
	}

	bool Engine::Update() {
		return true;
	}

	bool Engine::Init() {
		g_RenderBackend->Init();
		g_WindowSystem->Init();
		g_InputSystem->Init();
		g_GUISystem->Init();

		return true;
	}

	bool Engine::Shutdown() {
		g_GUISystem->Shutdown();
		g_RenderBackend->Shutdown();
		g_WindowSystem->Shutdown();
		g_InputSystem->Shutdown();
		return true;
	}

	bool Engine::Run() {
		return true;
	}
	
	float Engine::GetDeltaTime()
	{
		return 0.0f;
	}

	std::string Engine::GetName()
	{
		return "Bob";
	}

	DX12RenderBackend* Engine::GetRenderBackend() {
		return g_RenderBackend.get();
	}

	RenderDevice* Engine::GetDevice() {
		return g_RenderBackend->GetRenderDevice();
	}



	WinWindowSystem* Engine::GetWindowSystem() {
		return g_WindowSystem.get();
	}

	InputSystem* Engine::GetInputSystem() {
		return g_InputSystem.get();
	}

	GUISystem* Engine::GetGUISystem() {
		return g_GUISystem.get();
	}
}

