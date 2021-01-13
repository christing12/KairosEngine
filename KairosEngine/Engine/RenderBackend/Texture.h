#pragma once


#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>

#include <RenderBackend/Resource.h>
#include <RenderBackend/DescriptorHeap.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;


// POD data struct -> initialization and everything inside TextureManager
struct Texture : GPUResource
{
	Uint32 Width;
	Uint32 Height;
	Uint32 Depth;
	DXGI_FORMAT Format;
	Uint32 MipLevels;

	DescriptorAllocation RTV;
	DescriptorAllocation DSV;
	DescriptorAllocation SRV;
	DescriptorAllocation UAV;
};

//class Texture : public Resource {
//public:
//	Texture(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState,
//		const D3D12_CLEAR_VALUE* clearVal = nullptr, const std::wstring& debugName = L"Texture");
//	Texture(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> initResource,
//		D3D12_RESOURCE_STATES initState);
//	Texture(RenderDevice* pDevice, Ref<class Image> img, DXGI_FORMAT format, Uint32 levels);
//	Texture(RenderDevice* pDevice);
//
//	static Ref<Texture> LoadFromAsset(RenderDevice* pDevice, const char* filename, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
//		Uint32 depth = 1, Uint32 mipLevels = 1, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
//
//	Uint32 Width() const { return m_Width; }
//	Uint32 Height() const { return m_Height; }
//	Uint32 MipLevels() const { return m_Desc.MipLevels; }
//
//	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(Uint32 mipSlice, Uint32 arraySlice = 0, Uint32 planeSlice = 0);
//
//
//	bool CheckSRVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE); }
//	bool CheckRTVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET); }
//	bool CheckDSVSupport() const { return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL); }
//	bool CheckUAVSupport() const
//	{
//		return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) &&
//			CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) &&
//			CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE);
//	}
//
//	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RTV.GetDescriptorHandle(); }
//	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSV.GetDescriptorHandle(); }
//	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRV.GetDescriptorHandle(); }
//	D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(Uint32 mip) const;
//protected:
//	void InitTextureViews();
//
//	int m_Width = 0;
//	int m_Height = 0;
//	int m_Channels = 0;
//
//	DescriptorAllocation m_RTV;
//	DescriptorAllocation m_DSV;
//	DescriptorAllocation m_SRV;
//	DescriptorAllocation m_UAV;
//
//	bool isCubeMap = false;
//};



KRS_END_NAMESPACE