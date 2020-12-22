#include "krspch.h"
#include "Texture.h"
#include "RenderDevice.h"
#include "CommandContext.h"

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#include <assimp/Importer.hpp>

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
		//m_Device->GetD3DDevice()->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTV.GetDescriptorHandle());
	}

	Texture::Texture(RenderDevice* pDevice, const char* filename)
		: Resource(pDevice)
	{
		//stbi_set_flip_vertically_on_load(true);

		unsigned char* img = stbi_load(filename, &m_Width, &m_Height, &m_Channels, 0);
		unsigned char* img2 = new unsigned char[m_Width * m_Height * 4];

		for (int i = 0; i < m_Width * m_Height; i++) {
			img2[4 * i] = img[3 * i];
			img2[4 * i + 1] = img[3 * i + 1];
			img2[4 * i + 2] = img[3 * i + 2];
			img2[4 * i + 3] = 255;
		}


		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

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

		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = img2;
		subresource.RowPitch = m_Width * 4;
		subresource.SlicePitch = m_Height;

		D3D12_SUBRESOURCE_DATA subresources = {
			subresource
		};

		CheckFeatureSupport();

		CommandContext::InitTexture(m_Device, *this, 1, &subresources);
		InitTextureViews();
		delete[] img2;
		stbi_image_free(img);
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