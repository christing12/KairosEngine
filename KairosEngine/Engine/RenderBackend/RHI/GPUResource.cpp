#include "krspch.h"
#include "GPUResource.h"
#include "ResourceFormat.h"
#include "RenderDevice.h"

using namespace Kairos;

namespace Kairos {
	
	GPUAddress GPUResource::GPUVirtualAdress() const
	{
		 return m_dResource->GetGPUVirtualAddress();
	}

	GPUResource::GPUResource(RenderDevice* pDevice, const ResourceFormat& format, HeapType type)
		: m_Device(pDevice)
	{
		m_Desc = format.GetDesc();
		m_State = format.ResourceState();


		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = m_Desc.Format;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3DHeapType(type));

		auto hr = pDevice->D3DDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&m_Desc,
			D3DResourceStates(m_State),
			m_State == ResourceState::DepthWrite ? &depthOptimizedClearValue : nullptr,
			IID_PPV_ARGS(&m_dResource)
		);

		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create ID3D12Resource from GPUResource constructor");
	}

	void GPUResource::SetDebugName(const std::string& name)
	{
		m_dResource->SetName(Filesystem::StringToWString(name).c_str());
	}

	GPUResource::GPUResource(RenderDevice* pDevice, const D3DResourcePtr& exisitingResource, Kairos::ResourceState state)
		: m_Device(pDevice)
		, m_dResource(exisitingResource)
		, m_Desc(exisitingResource->GetDesc())
		, m_State(state)
	{

	}

}
