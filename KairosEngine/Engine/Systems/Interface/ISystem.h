#pragma once

#include "Core/EngineCore.h"
#include "Core/ClassTemplates.h"


KRS_BEGIN_NAMESPACE(Kairos)

struct ISystemConfig {

};

class ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(ISystem);

	virtual bool Setup(ISystemConfig* config = nullptr) = 0;
	virtual bool Init() = 0;
	virtual bool Update() { return true; }

	virtual bool Shutdown() = 0;
};

KRS_END_NAMESPACE