#include "krspch.h"
#include "Resource.h"
#include "RenderDevice.h"

namespace Kairos {
	Resource::Resource(RenderDevice* pDevice)
		: m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_CPUMemory(nullptr)
		, m_Device(pDevice)
	{

	}

	Resource::Resource(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal)
		: m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_CPUMemory(nullptr)
		, m_Device(pDevice)
		, m_Desc(desc)
		, m_CurrState(initState)
	{
	}

	Resource::Resource(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc,
		ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initState)
		: m_Device(pDevice)
		, m_Desc(desc)
		, m_Resource(resource)
		, m_CurrState(initState)
		, m_GPUAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		, m_CPUMemory(nullptr)
	{

	}


	void Resource::SetName(const std::wstring& name)
	{
		if (m_Resource != nullptr) {
			m_Resource->SetName(name.c_str());
		}
	}

	Resource::~Resource()
	{
		m_Resource = nullptr;
		m_GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		if (m_CPUMemory != nullptr) {
			m_CPUMemory = nullptr;
		}
	}

	void Resource::CheckFeatureSupport()
	{
		KRS_CORE_ASSERT(m_Resource != nullptr, " resource is null, unable to check feature support ");
		m_FormatSupport.Format = m_Desc.Format;
		auto hr = m_Device->GetD3DDevice()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &m_FormatSupport,
			sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "FEWFE");
	}

	::D3D12_RESOURCE_DESC Resource::CreateBufferDesc(uint32_t bufferSize)
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Alignment = 0;
		bufferDesc.Width = bufferSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		return bufferDesc;
	}

}