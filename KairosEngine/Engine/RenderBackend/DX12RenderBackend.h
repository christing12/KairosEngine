#pragma once

#include "Systems/Interface/IRenderBackend.h"


// Provides Video Memory Query, Adapter Support, Render Device, Render Graph implementation for D3D12
// RenderDoc & PIX integration validation layers 

KRS_BEGIN_NAMESPACE(Kairos)

class DX12RenderBackend {
public:

	KRS_INTERFACE_NON_COPYABLE(DX12RenderBackend);
	bool Setup(ISystemConfig* config);
	bool Init();
	bool Update();
	bool Shutdown();
	RenderDevice* GetRenderDevice();

};


KRS_END_NAMESPACE