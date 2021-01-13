#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include <wrl.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class LinearAllocator;

// POD data struct 
struct GPUResource {
	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress;
	Microsoft::WRL::ComPtr<ID3D12Resource> NativeResource;
	D3D12_RESOURCE_STATES ResourceState;
};

//
//class Resource {
//public:
//	using NativeResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
//
//	friend class LinearAllocator;
//	Resource(RenderDevice* pDevice);
//	// initializing a new d3d12 resource
//	Resource(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal = nullptr);
//	// for initializing resource with already existing resource
//	Resource(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc,
//		Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState);
//
//	friend class CommandContext;
//	friend class GraphicsContext;
//	friend class RenderDevice;
//
//	void SetName(const std::wstring& name);
//	virtual ~Resource();
//
//	ID3D12Resource* GetResource() { return m_Resource.Get(); }
//	const ID3D12Resource* GetResource() const { return m_Resource.Get(); }
//
// D3D12_RESOURCE_DESC GetDesc() const { return m_Desc; }
//
//	D3D12_RESOURCE_STATES GetCurrState() const { return m_CurrState; }
//protected:
//	void CheckFeatureSupport();
//	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT1 formatSupport) const { return (m_FormatSupport.Support1 & formatSupport) != 0; }
//	bool CheckFormatSupport(D3D12_FORMAT_SUPPORT2 formatSupport) const { return (m_FormatSupport.Support2 & formatSupport) != 0; }
//	static D3D12_RESOURCE_DESC CreateBufferDesc(uint32_t bufferSize);
//
//	RenderDevice* m_Device;
//	NativeResourcePtr m_Resource;
//
//	D3D12_RESOURCE_DESC m_Desc;
//	D3D12_RESOURCE_STATES m_CurrState;
//
//	D3D12_FEATURE_DATA_FORMAT_SUPPORT m_FormatSupport;
//	D3D12_GPU_VIRTUAL_ADDRESS m_GPUAddress;
//	void* m_CPUMemory;
//};

KRS_END_NAMESPACE