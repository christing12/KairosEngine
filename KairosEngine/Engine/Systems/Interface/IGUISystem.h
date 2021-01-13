#pragma once

#include "ISystem.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct IGUISystemConfig : public ISystemConfig {

};

class IGUISystem : public ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(IGUISystem);

	virtual void Render(class GraphicsContext& context) = 0;
};



KRS_END_NAMESPACE