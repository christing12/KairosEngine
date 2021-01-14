#include "krspch.h"
#include "Texture.h"
#include "RenderDevice.h"
#include "CommandContext.h"

#include <TextureAsset.h>



namespace Kairos
{
	TextureManager::TextureManager(RenderDevice* pDevice)
		: m_Device(pDevice)
	{
	}

	RenderHandle TextureManager::CreateTexture(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clear)
	{
		ComPtr<ID3D12Resource> nativeResource;
		// create D3D Resource
		auto hr = m_Device->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc,
			state,
			nullptr,
			IID_PPV_ARGS(&nativeResource)
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create Texture Resource");

		Texture tex;
		tex.Width = desc.Width;
		tex.Height = desc.Height;
		tex.Format = desc.Format;
		tex.Depth = desc.DepthOrArraySize;
		tex.MipLevels = desc.MipLevels;
		tex.GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		tex.ResourceState = state;
		tex.NativeResource = nativeResource;
		tex.Dimension = desc.Dimension;

		InitTextureViews(tex);

		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.push_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return handle;
	}

	RenderHandle TextureManager::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, D3D12_RESOURCE_STATES state)
	{
		D3D12_RESOURCE_DESC desc = nativeResource->GetDesc();

		Texture tex;
		tex.Width = desc.Width;
		tex.Height = desc.Height;
		tex.Format = desc.Format;
		tex.Depth = desc.DepthOrArraySize;
		tex.MipLevels = desc.MipLevels;
		tex.GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; //nativeResource->GetGPUVirtualAddress();
		tex.ResourceState = state;
		tex.NativeResource = nativeResource;
		tex.Dimension = desc.Dimension;

		InitTextureViews(tex);

		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.push_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return handle;
	}

	RenderHandle TextureManager::CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth, Uint32 mipLevels, D3D12_RESOURCE_FLAGS flags)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(filename, file);
		if (!loaded) {
			KRS_CORE_ERROR("error when loading mesh");
			return RenderHandle{};
		}
		Texture texture;
		assets::TextureInfo textureInfo = assets::read_texture_info(&file);
		texture.Width = textureInfo.pixelSize[0];
		texture.Height = textureInfo.pixelSize[1];
		
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension				= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width					= texture.Width;
		desc.Height					= texture.Height;
		desc.DepthOrArraySize		= depth;
		desc.MipLevels				= mipLevels;
		desc.Format					= format;
		desc.SampleDesc.Count		= 1;
		desc.Flags					= flags;
		

		ComPtr<ID3D12Resource> nativeResource;
		auto hr = m_Device->GetD3DDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&nativeResource)
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating a resource");

		Texture tex;
		tex.Width = desc.Width;
		tex.Height = desc.Height;
		tex.Format = desc.Format;
		tex.Depth = desc.DepthOrArraySize;
		tex.MipLevels = desc.MipLevels;
		tex.GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		tex.ResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
		tex.NativeResource = nativeResource;
		tex.Dimension = desc.Dimension;
		
		//texture.CheckFeatureSupport();
		
		char* data = new char[textureInfo.textureSize];
		assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), data);
				
		const D3D12_SUBRESOURCE_DATA subresourceData[] = {
			{ reinterpret_cast<void*>(data), textureInfo.textureSize / texture.Height }
		};
		
		m_Device->TransientCommand([&](CommandContext& ctx) {
			ctx.TransitionResource(tex, D3D12_RESOURCE_STATE_COPY_DEST, true);
			ctx.InitTexture(tex, _countof(subresourceData), subresourceData);
			ctx.TransitionResource(tex, D3D12_RESOURCE_STATE_COMMON);
			
		});

		InitTextureViews(texture);
		
		delete[] data;

		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.push_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return handle;
	}





	void TextureManager::InitTextureViews(Texture& texture)
	{
		if ((texture.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) {
			KRS_CORE_INFO("Creating Texture Render Target");
			texture.RTV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_Device->GetD3DDevice()->CreateRenderTargetView(texture.NativeResource.Get(), nullptr, texture.RTV.GetDescriptorHandle());
		}

		if ((texture.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {

			texture.DSV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
			ZeroMemory(&dsv, sizeof(dsv));

			dsv.Format = texture.Format;
			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsv.Texture2D.MipSlice = 0;
			dsv.Flags = D3D12_DSV_FLAG_NONE;
			

			m_Device->GetD3DDevice()->CreateDepthStencilView(texture.NativeResource.Get(), &dsv, texture.DSV.GetDescriptorHandle());
		}

		if (!(texture.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
			texture.SRV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = texture.Format;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			switch (texture.Depth) {
			case 1:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = texture.MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				break;
			case 6:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = texture.MipLevels;
				break;
			default:
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.MipLevels = 1;
				srvDesc.Texture2DArray.FirstArraySlice = 0;
				srvDesc.Texture2DArray.ArraySize = texture.Depth;
				break;
			}

			m_Device->GetD3DDevice()->CreateShaderResourceView(texture.NativeResource.Get(), &srvDesc, texture.SRV.GetDescriptorHandle());
		}


		//if ((texture.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) {
		//	texture.UAV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, texture.MipLevels);
		//	for (int i = 0; i < texture.MipLevels; i++) {
		//		auto uavDesc = GetUAVDesc(texture, i);
		//		m_Device->GetD3DDevice()->CreateUnorderedAccessView(texture.NativeResource.Get(), nullptr, &uavDesc, texture.UAV.GetDescriptorHandle(i));
		//	}
		//}
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC TextureManager::GetUAVDesc(const Texture& texture, Uint32 mipSlice, Uint32 arraySlice, Uint32 planeSlice)
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = texture.Format;
		switch (texture.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (texture.Depth > 1) {
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDesc.Texture2DArray.ArraySize = texture.Depth - arraySlice;
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


	Texture* TextureManager::GetUnreferencedTexture(RenderHandle handle)
	{
		Uint64 index = IndexFromHandle(handle);
		return &m_AllocatedTextures[index];
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = IndexFromHandle(handle);
		return m_AllocatedTextures[index].SRV.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetDSV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = IndexFromHandle(handle);
		return m_AllocatedTextures[index].DSV.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetRTV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = IndexFromHandle(handle);
		return m_AllocatedTextures[index].RTV.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetUAV(RenderHandle handle, Uint32 index)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 dex = IndexFromHandle(handle);
		return m_AllocatedTextures[dex].UAV.GetDescriptorHandle(index);
	}

	Uint64 TextureManager::IndexFromHandle(const RenderHandle& handle)
	{
		constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
		return Uint64((handle.handle & resourceMask) >> 8);
	}

}
//
//namespace Kairos {
//	Texture::Texture(RenderDevice* pDevice, D3D12_RESOURCE_DESC desc, D3D12_RESOURCE_STATES initState,
//		const D3D12_CLEAR_VALUE* clearVal, const std::wstring& debugName)
//		: Resource(pDevice, desc, initState, clearVal)
//	{
//		m_Width = desc.Width;
//		m_Height = desc.Height;
//		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
//		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
//			&heapProps,
//			D3D12_HEAP_FLAG_NONE,
//			&desc,
//			initState,
//			clearVal,
//			IID_PPV_ARGS(&m_Resource)
//		);
//		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating texture resource");
//		m_Resource->SetName(debugName.c_str());
//		CheckFeatureSupport();
//		InitTextureViews();
//	}
//
//
//	Texture::Texture(RenderDevice* pDevice, ComPtr<ID3D12Resource> initResource,
//		D3D12_RESOURCE_STATES initState)
//		: Resource(pDevice, initResource->GetDesc(), initResource, initState)
//	{
//		CheckFeatureSupport();
//		InitTextureViews();
//	}
//
//
//	Texture::Texture(RenderDevice* pDevice, Ref<Image> img, DXGI_FORMAT format, Uint32 levels)
//		: Resource(pDevice)
//		, m_Height(img->Height())
//		, m_Width(img->Width())
//	{
//		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
//		m_Desc = CD3DX12_RESOURCE_DESC::Tex2D(format, m_Width, m_Height);
//		m_Desc.MipLevels = 1;
//
//
//		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
//			&heapProps,
//			D3D12_HEAP_FLAG_NONE,
//			&m_Desc,
//			D3D12_RESOURCE_STATE_COPY_DEST,
//			nullptr,
//			IID_PPV_ARGS(&m_Resource)
//		);
//
//		KRS_CORE_ASSERT(SUCCEEDED(hr) && m_Resource != nullptr, "Issue with creating a resource");
//		CheckFeatureSupport();
//
//		const D3D12_SUBRESOURCE_DATA data[] = {
//			{ img->Pixels<void>(), img->Pitch() }
//		};
//		CommandContext::InitTexture(m_Device, *this, 1, data);
//
//		InitTextureViews();
//	}
//
//	Texture::Texture(RenderDevice* pDevice)
//		: Resource(pDevice)
//	{
//	}
//
//	Ref<Texture> Texture::LoadFromAsset(RenderDevice* pDevice, const char* filename, DXGI_FORMAT format, Uint32 depth /* = 1*/ , Uint32 mipLevels /* = 1 */, 
//		D3D12_RESOURCE_FLAGS flags)
//	{
//		assets::AssetFile file;
//		bool loaded = assets::load_binaryfile(filename, file);
//		if (!loaded) {
//			KRS_CORE_ERROR("error when loading mesh");
//			return false;
//		}
//		Ref<Texture> texture = CreateRef<Texture>(pDevice);
//		assets::TextureInfo textureInfo = assets::read_texture_info(&file);
//		texture->m_Width = textureInfo.pixelSize[0];
//		texture->m_Height = textureInfo.pixelSize[1];
//
//		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
//
//		D3D12_RESOURCE_DESC desc = {};
//		desc.Dimension				= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//		desc.Width					= texture->m_Width;
//		desc.Height					= texture->m_Height;
//		desc.DepthOrArraySize		= depth;
//		desc.MipLevels				= mipLevels;
//		desc.Format					= format;
//		desc.SampleDesc.Count		= 1;
//		desc.Flags					= flags;
//
//		texture->m_Desc = desc;
//
//		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
//			&heapProps,
//			D3D12_HEAP_FLAG_NONE,
//			&texture->m_Desc,
//			D3D12_RESOURCE_STATE_COPY_DEST,
//			nullptr,
//			IID_PPV_ARGS(&texture->m_Resource)
//		);
//
//		KRS_CORE_ASSERT(SUCCEEDED(hr) && texture->m_Resource != nullptr, "Issue with creating a resource");
//		texture->CheckFeatureSupport();
//
//		char* data = new char[textureInfo.textureSize];
//		assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), data);
//
//		
//		const D3D12_SUBRESOURCE_DATA subresourceData[] = {
//			{ reinterpret_cast<void*>(data), textureInfo.textureSize / texture->m_Height }
//		};
//
//		CommandContext::InitTexture(pDevice, *texture.get(), 1, subresourceData);
//
//		texture->InitTextureViews();
//
//		delete[] data;
//		return texture;
//	}
//
//
//
//	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUAV(Uint32 mip) const
//	{
//		//KRS_CORE_ASSERT(mip < m_UAV.G)
//		return m_UAV.GetDescriptorHandle(mip);
//	}
//
//	void Texture::InitTextureViews()
//	{
//		if ((m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) && CheckRTVSupport()) {
//			KRS_CORE_INFO("Creating Texture Render Target");
//			m_RTV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//			m_Device->GetD3DDevice()->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTV.GetDescriptorHandle());
//		}
//
//		if ((m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) && CheckDSVSupport()) {
//
//			m_DSV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//
//			D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
//			ZeroMemory(&dsv, sizeof(dsv));
//
//			dsv.Format = m_Desc.Format;
//			dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
//			dsv.Texture2D.MipSlice = 0;
//			dsv.Flags = D3D12_DSV_FLAG_NONE;
//			
//
//			m_Device->GetD3DDevice()->CreateDepthStencilView(m_Resource.Get(), &dsv, m_DSV.GetDescriptorHandle());
//		}
//
//		if (!(m_Desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) && CheckSRVSupport()) {
//			m_SRV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//			srvDesc.Format = m_Desc.Format;
//			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//			switch (m_Desc.DepthOrArraySize) {
//			case 1:
//				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//				srvDesc.Texture2D.MipLevels = m_Desc.MipLevels;
//				srvDesc.Texture2D.MostDetailedMip = 0;
//				break;
//			case 6:
//				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
//				srvDesc.TextureCube.MostDetailedMip = 0;
//				srvDesc.TextureCube.MipLevels = m_Desc.MipLevels;
//				break;
//			default:
//				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
//				srvDesc.Texture2DArray.MostDetailedMip = 0;
//				srvDesc.Texture2DArray.MipLevels = 1;
//				srvDesc.Texture2DArray.FirstArraySlice = 0;
//				srvDesc.Texture2DArray.ArraySize = m_Desc.DepthOrArraySize;
//				break;
//			}
//
//			m_Device->GetD3DDevice()->CreateShaderResourceView(m_Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle());
//		}
//
//
//		if ((m_Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) && CheckUAVSupport()) {
//			m_UAV = m_Device->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_Desc.MipLevels);
//			for (int i = 0; i < m_Desc.MipLevels; i++) {
//				auto uavDesc = GetUAVDesc(i);
//				m_Device->GetD3DDevice()->CreateUnorderedAccessView(m_Resource.Get(), nullptr, &uavDesc, m_UAV.GetDescriptorHandle(i));
//			}
//		}
//	}
//
//	D3D12_UNORDERED_ACCESS_VIEW_DESC Texture::GetUAVDesc(Uint32 mipSlice, Uint32 arraySlice /* = 0 */, Uint32 planeSlice /* = 0 */)
//	{
//
//		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
//		uavDesc.Format = m_Desc.Format;
//		switch (m_Desc.Dimension)
//		{
//		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
//			if (m_Desc.DepthOrArraySize > 1) {
//				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
//				uavDesc.Texture2DArray.ArraySize = m_Desc.DepthOrArraySize - arraySlice;
//				uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
//				uavDesc.Texture2DArray.PlaneSlice = planeSlice;
//				uavDesc.Texture2DArray.MipSlice = mipSlice;
//			}
//			else {
//				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
//				uavDesc.Texture2D.PlaneSlice = planeSlice;
//				uavDesc.Texture2D.MipSlice = mipSlice;
//			}
//			break;
//		default:
//			KRS_CORE_ERROR("INVALID DIMENSION???");
//			break;
//		}
//
//		return uavDesc;
//	}
//
//}

