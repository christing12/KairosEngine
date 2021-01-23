#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>
#include "GraphicsTypes.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class GPUResource {
public:
	using D3DResourcePtr = Microsoft::WRL::ComPtr<ID3D12Resource>;
	using ResourceDesc = D3D12_RESOURCE_DESC;

	GPUResource() = default;
	// constructor for existing resource
	GPUResource(RenderDevice* pDevice, const D3DResourcePtr& exisitingResource, ResourceState state = ResourceState::Common);
	// construct for creating the id3d12resource
	GPUResource(RenderDevice* pDevice, const class ResourceFormat& format, HeapType type = HeapType::Default);

	~GPUResource() = default;
	void SetDebugName(const std::string& name);
	
protected:


protected:
	RenderDevice* m_Device = nullptr;
	D3DResourcePtr m_dResource = nullptr;
	GPUAddress m_GPUAddress;
	ResourceDesc m_Desc{};
	ResourceState m_State = ResourceState::Common;

public:
	GPUAddress GPUVirtualAdress() const;
	void SetResourceState(ResourceState newState) { m_State = newState; }
	inline const ResourceDesc& GetDesc() const { return m_Desc; }
	inline ID3D12Resource* D3DResource() { return m_dResource.Get(); }
	inline const ID3D12Resource* D3DResource() const { return m_dResource.Get(); }
	inline ResourceState ResourceState() const { return m_State; }
};

KRS_END_NAMESPACE