#pragma once

#include "Core/EngineCore.h"
#include "Core/ClassTemplates.h"


KRS_BEGIN_NAMESPACE(Kairos)


class IEngine {
public:
	KRS_INTERFACE_NON_COPYABLE(IEngine);

	virtual bool Setup(void* appHook, void* extraHook, char* cmdLine) = 0;
	virtual bool Init() = 0;

	virtual bool Update() = 0;
	virtual bool Shutdown() = 0;

	virtual bool Run() = 0;
	virtual float GetDeltaTime() = 0;

	virtual std::string GetName() = 0;

public:
	virtual class DX12RenderBackend* GetRenderBackend() = 0;
	virtual class IWindowSystem* GetWindowSystem() = 0;
	virtual class IEventSystem* GetEventSystem() = 0;
	virtual class IInputSystem* GetInputSystem() = 0;
	virtual class IGUISystem* GetGUISystem() = 0;
	virtual class ICVarSystem* GetCVarSystem() = 0;
	virtual class RenderDevice* GetDevice() = 0;
};

KRS_END_NAMESPACE