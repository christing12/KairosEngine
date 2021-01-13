#pragma once

#include "Interface/IWindowSystem.h"

KRS_BEGIN_NAMESPACE(Kairos)


class WinWindowSystem : public IWindowSystem {
public:

	bool Setup(ISystemConfig* config) override final;
	bool Init() override final;

	bool Update() override final;
	bool Shutdown() override final;

	virtual IWindowSurface* GetWindowSurface() override final;
	virtual void SetEventCallback(const EventCallbackFn& callback) override final;


	HWND GetHWND();
	HINSTANCE GetHINSTANCE();
	bool SetHWND(HWND hwnd);
	LPCWSTR GetAppName();

};


KRS_END_NAMESPACE