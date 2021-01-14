#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include <wrl.h>

KRS_BEGIN_NAMESPACE(Kairos)

// POD data struct 
struct GPUResource
{
	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
	Microsoft::WRL::ComPtr<ID3D12Resource> NativeResource;
	D3D12_RESOURCE_STATES ResourceState;
};
KRS_END_NAMESPACE