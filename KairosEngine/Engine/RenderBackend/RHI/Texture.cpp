#include "krspch.h"
#include "Texture.h"
#include "ResourceFormat.h"
#include "RenderDevice.h"
#include "DX12Util.h"

namespace Kairos {
	Texture::Texture(RenderDevice* pDevice, Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, Kairos::ResourceState state)
		: GPUResource(pDevice, nativeResource, state)
	{
		m_Properties.Width = m_Desc.Width;
		m_Properties.Height = m_Desc.Height;
		m_Properties.Depth = m_Desc.DepthOrArraySize;
		m_Properties.MipLevels = m_Desc.MipLevels;
	}

	Texture::Texture(RenderDevice* pDevice, const TextureProperties& props, Kairos::ResourceState state)
		: GPUResource(pDevice, ResourceFormat(props, state))
		, m_Properties(props)
	{}

	// shader visible
	Descriptor Texture::GetSRDescriptor()
	{
		if (m_SRVDescriptor == nullptr && ((m_Desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0))
		{
			Descriptor dest = m_Device->AllocateDescriptor(DescriptorType::SRV, 1, true).GetDescriptor(0);
			

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = m_Desc.Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (m_Desc.DepthOrArraySize) {
			case 1:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = m_Desc.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				break;
			case 6:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = m_Desc.MipLevels;
				break;
			default:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = 1;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ArraySize = m_Desc.DepthOrArraySize;
				break;
			}
			m_Device->D3DDevice()->CreateShaderResourceView(m_dResource.Get(), &srvDesc, dest.CPUHandle());
			
			m_SRVDescriptor = CreateScope<Descriptor>(std::move(dest));
		}

		return *m_SRVDescriptor.get();
	}

	// non shader visible
	Descriptor Texture::GetDSVDescriptor()
	{
		if (m_DSVDescriptor == nullptr && (m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {
			Descriptor dest = m_Device->AllocateDescriptor(DescriptorType::DSV, 1, false).GetDescriptor(0);
			
			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
			ZeroMemory(&dsv, sizeof(dsv));

			dsv.Format = m_Desc.Format;
			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = D3D12_DSV_FLAG_NONE;

			m_Device->D3DDevice()->CreateDepthStencilView(m_dResource.Get(), &dsv, dest.CPUHandle());
			m_DSVDescriptor = CreateScope<Descriptor>(std::move(dest));
		}

		return *m_DSVDescriptor.get();
	}

	Descriptor Texture::GetRTVDescriptor(Uint32 offset)
	{
		if (m_RTVDescriptor == nullptr && (m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) {
			DescriptorRange dest = m_Device->AllocateDescriptor(DescriptorType::RTV, 1, false);
			m_Device->D3DDevice()->CreateRenderTargetView(m_dResource.Get(), nullptr, dest.GetDescriptor(0).CPUHandle());
			m_RTVDescriptor = CreateScope<DescriptorRange>(std::move(dest));
		}

		return m_RTVDescriptor->GetDescriptor(offset);
	}

	Descriptor Texture::GetUAVDescriptor(Uint32 offset)
	{
		if (m_UAVDescriptors == nullptr) {
			KRS_CORE_ASSERT(m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, "This Resource is not allowed to have a UAV");
			DescriptorRange dest = m_Device->AllocateDescriptor(DescriptorType::UAV, m_Desc.MipLevels, true);

			for (int mip = 0; mip < m_Desc.MipLevels; ++mip)
			{
				auto uavDesc = Texture::GetUAVDesc(this, mip);
				m_Device->D3DDevice()->CreateUnorderedAccessView(m_dResource.Get(), nullptr, &uavDesc, dest.GetDescriptor(mip).CPUHandle());
			}
			m_UAVDescriptors = CreateScope<DescriptorRange>(std::move(dest));
		}
		return m_UAVDescriptors->GetDescriptor(offset);
	}


	D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::GetUAVDesc(const Texture* texture, Uint32 mipSlice, Uint32 arraySlice, Uint32 planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texture->m_Desc.Format;
		switch (texture->m_Desc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (texture->m_Desc.DepthOrArraySize > 1) {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = texture->m_Desc.DepthOrArraySize - arraySlice;
				uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
				uavDesc.Texture2DArray.PlaneSlice = planeSlice;
				uavDesc.Texture2DArray.MipSlice = mipSlice;
			}
			else {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.PlaneSlice = planeSlice;
				uavDesc.Texture2D.MipSlice = mipSlice;
			}
			break;
		default:
			KRS_CORE_ERROR("INVALID DIMENSION???");
			break;
		}

		return uavDesc;
	}
}