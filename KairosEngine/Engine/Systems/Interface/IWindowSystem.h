#pragma once


#include <Core/EngineCore.h>
#include <Core/ClassTemplates.h>

#include <Core/Events/Event.h>

#include "ISystem.h"
#include "IWindowSurface.h"

KRS_BEGIN_NAMESPACE(Kairos)


struct IWindowSystemConfig : public ISystemConfig {
	void* AppHook;
	void* ExtraHook;
};

class IWindowSystem : public ISystem {
public:
	using EventCallbackFn = std::function<void(Event&)>;

	KRS_INTERFACE_NON_COPYABLE(IWindowSystem);

	virtual IWindowSurface* GetWindowSurface() = 0;

	virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

};

KRS_END_NAMESPACE