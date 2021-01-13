#pragma once


#include "ISystem.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct IWindowSurfaceConfig : public ISystemConfig {
	void* hInstance;
	void* hWND;
	void* WndProc;
};

class IWindowSurface : public ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(IWindowSurface);


};

KRS_END_NAMESPACE