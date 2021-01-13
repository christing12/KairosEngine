#pragma once

#include "ISystem.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct IEventSystemConfig : ISystemConfig {

};

class IEventSystem : ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(IEventSystem);


};

KRS_END_NAMESPACE