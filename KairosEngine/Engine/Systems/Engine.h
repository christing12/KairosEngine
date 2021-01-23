#pragma once

//#include "Interface/IEngine.h

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>

#include "GUISystem.h"
#include "InputSystem.h"
#include "RenderBackend/DX12RenderBackend.h"

KRS_BEGIN_NAMESPACE(Kairos)

class DX12RenderBackend;
class WinWindowSystem;
class EventSystem;
class InputSystem;
class GUISystem;


class Engine {
public:
	bool Setup(void* appHook, void* extraHook, char* cmdLine);
	bool Init();

	bool Update();
	bool Shutdown();

	bool Run();
	float GetDeltaTime();

	std::string GetName();

public:
	DX12RenderBackend* GetRenderBackend();
	WinWindowSystem* GetWindowSystem();
	InputSystem* GetInputSystem();
	GUISystem* GetGUISystem();
	class RenderDevice* GetDevice();

private:
	Scope<DX12RenderBackend> g_RenderBackend;
	Scope<WinWindowSystem> g_WindowSystem;
	Scope<InputSystem> g_InputSystem;
	Scope<GUISystem> g_GUISystem;
};

KRS_END_NAMESPACE