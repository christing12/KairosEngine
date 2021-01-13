

#include "Interface/IWindowSurface.h"

KRS_BEGIN_NAMESPACE(Kairos)

class D3DWindowSurface : public IWindowSurface {
public:
	virtual bool Setup(ISystemConfig* config) override final;
	virtual bool Init() override final;
	virtual bool Update() override final;
	virtual bool Shutdown() override final;
};

KRS_END_NAMESPACE