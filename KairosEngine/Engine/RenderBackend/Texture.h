#pragma once


#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>
#include "GraphicsTypes.h"


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
	D3D12_RESOURCE_FLAGS Flags;
	D3D12_RESOURCE_DIMENSION Dimension;

	DescriptorAllocation RTV;
	DescriptorAllocation DSV;
	DescriptorAllocation SRV;
	DescriptorAllocation UAV;
};

class TextureManager {
public:
	KRS_CLASS_DEFAULT(TextureManager);

	TextureManager(RenderDevice* pDevice);

	RenderHandle CreateTexture(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clear = nullptr);
	RenderHandle CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, D3D12_RESOURCE_STATES state);
	RenderHandle CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth = 1, Uint32 levels = 1, D3D12_RESOURCE_FLAGS = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);


	Texture* GetUnreferencedTexture(RenderHandle handle);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(RenderHandle handle);
	D3D12_CPU_DESCRIPTOR_HANDLE GetUAV(RenderHandle handle, Uint32 index = 0);


private:
	
	RenderDevice* m_Device;

	Uint64 IndexFromHandle(const RenderHandle& handle);
	std::vector<Texture> m_AllocatedTextures;
	void InitTextureViews(Texture& texture);
	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const Texture& texture, Uint32 mipSlice, Uint32 arraySlice = 0, Uint32 planeSlice = 0);
};


KRS_END_NAMESPACE