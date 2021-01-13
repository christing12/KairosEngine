#pragma once


#include "Interface/IGUISystem.h"


KRS_BEGIN_NAMESPACE(Kairos)

class GUISystem : public IGUISystem {
public:
	virtual bool Setup(ISystemConfig* config) override;
	virtual bool Init() override;
	virtual bool Update() override;
	virtual bool Shutdown() override;

	virtual void Render(class GraphicsContext& context) override;
};


KRS_END_NAMESPACE