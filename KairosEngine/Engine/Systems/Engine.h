#pragma once

#include "Interface/IEngine.h"

KRS_BEGIN_NAMESPACE(Kairos)

class Engine : public IEngine {
public:
	virtual bool Setup(void* appHook, void* extraHook, char* cmdLine) override final;
	virtual bool Init() override final;

	virtual bool Update() override final;
	virtual bool Shutdown() override final;

	virtual bool Run() override final;
	virtual float GetDeltaTime() override final;

	virtual std::string GetName() override final;

public:
	virtual class DX12RenderBackend* GetRenderBackend() override final;
	virtual class IWindowSystem* GetWindowSystem() override final;
	virtual class IEventSystem* GetEventSystem() override final;
	virtual class IInputSystem* GetInputSystem() override final;
	virtual class IGUISystem* GetGUISystem() override final;
	virtual class ICVarSystem* GetCVarSystem() override final;
	virtual class RenderDevice* GetDevice() override final;
};

KRS_END_NAMESPACE