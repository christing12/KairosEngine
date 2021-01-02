#include "krspch.h"
#include "Texture.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "Image.h"

#include <TextureAsset.h>

namespace Kairos {
	Texture::Texture(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* clearVal)
		: Resource(pDevice, desc, initState, clearVal)
	{
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			initState,
			clearVal,
			IID_PPV_ARGS(&m_Resource)
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating texture resource");
		CheckFeatureSupport();
		InitTextureViews();
	}


	Texture::Texture(RenderDevice* pDevice, ComPtr<ID3D12Resource> initResource,
		D3D12_RESOURCE_STATES initState)
		: Resource(pDevice, initResource->GetDesc(), initResource, initState)
	{
		CheckFeatureSupport();
		InitTextureViews();
	}


	Texture::Texture(RenderDevice* pDevice, Ref<Image> img, DXGI_FORMAT format, Uint32 levels)
		: Resource(pDevice)
		, m_Height(img->Height())
		, m_Width(img->Width())
	{
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		m_Desc = CD3DX12_RESOURCE_DESC::Tex2D(format, m_Width, m_Height);
		m_Desc.MipLevels = 1;


		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&m_Desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_Resource)
		);

		KRS_CORE_ASSERT(SUCCEEDED(hr) && m_Resource != nullptr, "Issue with creating a resource");
		CheckFeatureSupport();

		const D3D12_SUBRESOURCE_DATA data[] = {
			{ img->Pixels<void>(), img->Pitch() }
		};
		CommandContext::InitTexture(m_Device, *this, 1, data);

		InitTextureViews();
	}

	Texture::Texture(RenderDevice* pDevice)
		: Resource(pDevice)
	{
	}

	Ref<Texture> Texture::LoadFromAsset(RenderDevice* pDevice, const char* filename, DXGI_FORMAT format)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(filename, file);
		if (!loaded) {
			KRS_CORE_ERROR("error when loading mesh");
			return false;
		}
		Ref<Texture> texture = CreateRef<Texture>(pDevice);
		assets::TextureInfo textureInfo = assets::read_texture_info(&file);
		texture->m_Width = textureInfo.pixelSize[0];
		texture->m_Height = textureInfo.pixelSize[1];

		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		texture->m_Desc = CD3DX12_RESOURCE_DESC::Tex2D(format, texture->m_Width, texture->m_Height);
		texture->m_Desc.MipLevels = 1;

		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texture->m_Desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&texture->m_Resource)
		);

		KRS_CORE_ASSERT(SUCCEEDED(hr) && texture->m_Resource != nullptr, "Issue with creating a resource");
		texture->CheckFeatureSupport();

		char* data = new char[textureInfo.textureSize];
		assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), data);

		
		const D3D12_SUBRESOURCE_DATA subresourceData[] = {
			{ reinterpret_cast<void*>(data), textureInfo.textureSize / texture->m_Height }
		};

		CommandContext::InitTexture(pDevice, *texture.get(), 1, subresourceData);

		texture->InitTextureViews();

		delete[] data;
		return texture;
	}

	void Texture::InitTextureViews()
	{
		if ((m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && CheckRTVSupport()) {
			KRS_CORE_INFO("Creating Texture Render Target");
			m_RTV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_Device->GetD3DDevice()->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTV.GetDescriptorHandle());
		}

		if ((m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && CheckDSVSupport()) {

			m_DSV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
			ZeroMemory(&dsv, sizeof(dsv));

			dsv.Format = m_Desc.Format;
			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = D3D12_DSV_FLAG_NONE;
			

			m_Device->GetD3DDevice()->CreateDepthStencilView(m_Resource.Get(), &dsv, m_DSV.GetDescriptorHandle());
		}

		if (!(m_Desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) && CheckSRVSupport()) {
			m_SRV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_Device->GetD3DDevice()->CreateShaderResourceView(m_Resource.Get(), nullptr,
				m_SRV.GetDescriptorHandle());
		}
	}
}