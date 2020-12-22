#pragma once


#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>

#include <Graphics/Core/Resource.h>
#include <Graphics/Core/DescriptorHeap.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class Texture : public Resource {
public:
	Texture(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal = nullptr);
	Texture(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> initResource,
		D3D12_RESOURCE_STATES initState);
	Texture(RenderDevice* pDevice, const char* filename);

	Uint32 Width() const { return m_Width; }
	Uint32 Height() const { return m_Height; }

	bool CheckSRVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE); }
	bool CheckRTVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET); }
	bool CheckDSVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RTV.GetDescriptorHandle(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSV.GetDescriptorHandle(); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRV.GetDescriptorHandle(); }
protected:
	void InitTextureViews();

	int m_Width = 0;
	int m_Height = 0;
	int m_Channels = 0;

	DescriptorAllocation m_RTV;
	DescriptorAllocation m_DSV;
	DescriptorAllocation m_SRV;
	DescriptorAllocation m_UAV;
};

KRS_END_NAMESPACE