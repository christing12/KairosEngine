#include "krspch.h"
#include "Engine.h"

#include "Interface/IEventSystem.h"
#include "WinWindowSystem.h"


#include "RenderBackend/DX12RenderBackend.h"
#include "WinWindowSystem.h"

#include "InputSystem.h"
#include "GUISystem.h"
#include "CVarSystem.h"


Kairos::IEngine* g_Engine;


#define GetSystemDef( system ) \
I##system * Engine::Get##system() \
{ \
	return g_##system.get(); \
} \


namespace Kairos {

	namespace EngineGlobals {

		bool Setup(void* appHook, void* extraHook, char* cmdLine);
		bool Init();
		bool Update();
		bool Shutdown();
		bool Run();

		Scope<DX12RenderBackend> g_RenderBackend;
		Scope<IWindowSystem> g_WindowSystem;
		Scope<IEventSystem> g_EventSystem;
		Scope<IInputSystem> g_InputSystem;
		Scope<IGUISystem> g_GUISystem;
		Scope<ICVarSystem> g_CVarSystem;
	}

	bool EngineGlobals::Setup(void* appHook, void* extraHook, char* cmdLine) {
		// create systems then setup them up
		g_RenderBackend = CreateScope<DX12RenderBackend>();
		g_WindowSystem = CreateScope<WinWindowSystem>();
		g_InputSystem = CreateScope<InputSystem>();
		g_GUISystem = CreateScope<GUISystem>();
		g_CVarSystem = CreateScope<CVarSystem>();


		if (!g_CVarSystem->Setup()) {
			KRS_CORE_ERROR("Error with creating cvar system");
			return false;
		}

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

		if (!g_InputSystem->Setup()) {
			KRS_CORE_ERROR("Error with creating Input System");
			return false;
		}

		if (!g_GUISystem->Setup()) {
			KRS_CORE_ERROR("Error with Creating gui system");
			return false;
		}



		return true;
	}

	bool EngineGlobals::Update() {
		return true;
	}

	bool EngineGlobals::Init() {
		g_RenderBackend->Init();
		g_WindowSystem->Init();
		g_InputSystem->Init();
		g_GUISystem->Init();
		g_CVarSystem->Init();

		AutoCVar_Int CVAR_Test("test.int", "just a configurable int", 42);

		return true;
	}

	bool EngineGlobals::Shutdown() {
		g_RenderBackend->Shutdown();
		g_WindowSystem->Shutdown();
		g_InputSystem->Shutdown();
		g_GUISystem->Shutdown();
		g_CVarSystem->Shutdown();
		return true;
	}

	bool EngineGlobals::Run() {
		return true;
	}
	

	bool Engine::Setup(void* appHook, void* extraHook, char* cmdLine)
	{
		g_Engine = this;
		return EngineGlobals::Setup(appHook, extraHook,  cmdLine);
	}

	bool Engine::Init()
	{
		return EngineGlobals::Init();
	}

	bool Engine::Update()
	{
		return EngineGlobals::Update();
	}

	bool Engine::Shutdown()
	{
		return EngineGlobals::Shutdown();
	}

	bool Engine::Run()
	{
		return EngineGlobals::Run();
	}

	float Engine::GetDeltaTime()
	{
		return 0.0f;
	}

	std::string Engine::GetName()
	{
		return "Bob";
	}

	using namespace EngineGlobals;


	DX12RenderBackend* Engine::GetRenderBackend() {
		return g_RenderBackend.get();
	}

	RenderDevice* Engine::GetDevice() {
		return g_RenderBackend->GetRenderDevice();
	}


	GetSystemDef(WindowSystem);
	GetSystemDef(EventSystem);
	GetSystemDef(InputSystem);
	GetSystemDef(GUISystem);
	GetSystemDef(CVarSystem);

}

