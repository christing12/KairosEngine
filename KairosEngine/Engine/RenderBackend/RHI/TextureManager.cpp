#include "krspch.h"
#include "TextureManager.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "ResourceFormat.h"
#include "Texture.h"


#include <TextureAsset.h>



namespace Kairos
{
	TextureManager::TextureManager(RenderDevice* pDevice)
		: m_Device(pDevice)
	{
	}
	
	void TextureManager::Shutdown()
	{
		m_AllocatedTextures.clear();
	}

	TextureQueryResult TextureManager::CreateTexture(const TextureProperties& props, ResourceState state)
	{
		Texture* tex = new Texture(m_Device, props, state);

		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.emplace_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return TextureQueryResult{ handle, m_AllocatedTextures[index].get() };
	}

	TextureQueryResult TextureManager::CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> nativeResource, ResourceState state)
	{
		Texture* tex = new Texture(m_Device, nativeResource, state);

		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.emplace_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return TextureQueryResult{ handle, m_AllocatedTextures[index].get() };
	}

	TextureQueryResult TextureManager::CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth, Uint32 mipLevels)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(filename, file);
		if (!loaded) {
			KRS_CORE_ERROR("error when loading mesh");
			return TextureQueryResult{};
		}
		assets::TextureInfo textureInfo = assets::read_texture_info(&file);


		TextureProperties props = TextureProperties(textureInfo.pixelSize[0], textureInfo.pixelSize[1],
			format, TextureType::Tex2D, depth, mipLevels);

		//TODO: better resource states?
		Texture* tex = new Texture(m_Device, props, ResourceState::UnorderedAccess);

		char* data = new char[textureInfo.textureSize];
		assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), data);

		
		const D3D12_SUBRESOURCE_DATA subresourceData[] = {
			{ static_cast<void*>(data), textureInfo.textureSize / tex->Height(), textureInfo.textureSize }
		};

		m_Device->TransientCommand([&](CommandContext& ctx) {
			ctx.TransitionResource(*tex, ResourceState::CopyDest, true);
			ctx.InitTexture(*tex, _countof(subresourceData), subresourceData);
			ctx.TransitionResource(*tex, ResourceState::Common);
		});


		delete[] data;
		Uint64 index = m_AllocatedTextures.size();
		m_AllocatedTextures.emplace_back(tex);

		RenderHandle handle{ static_cast<Uint64>(RenderResourceType::Texture) | (index << 8) };
		return TextureQueryResult{ handle, m_AllocatedTextures[index].get() };
	}

	//D3D12_UNORDERED_ACCESS_VIEW_DESC TextureManager::GetUAVDesc(const Texture* texture, Uint32 mipSlice, Uint32 arraySlice, Uint32 planeSlice)
	//{
	//	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	//	uavDesc.Format = texture->Format;
	//	switch (texture->Dimension)
	//	{
	//	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
	//		if (texture->Depth > 1) {
	//			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	//			uavDesc.Texture2DArray.ArraySize = texture->Depth - arraySlice;
	//			uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
	//			uavDesc.Texture2DArray.PlaneSlice = planeSlice;
	//			uavDesc.Texture2DArray.MipSlice = mipSlice;
	//		}
	//		else {
	//			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//			uavDesc.Texture2D.PlaneSlice = planeSlice;
	//			uavDesc.Texture2D.MipSlice = mipSlice;
	//		}
	//		break;
	//	default:
	//		KRS_CORE_ERROR("INVALID DIMENSION???");
	//		break;
	//	}

	//	return uavDesc;
	//}


	Texture* TextureManager::GetUnreferencedTexture(RenderHandle handle)
	{
		Uint64 index = TextureIndexFromHandle(handle);
		return m_AllocatedTextures[index].get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = TextureIndexFromHandle(handle);
		return m_AllocatedTextures[index]->GetSRDescriptor().CPUHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetDSV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = TextureIndexFromHandle(handle);
		return m_AllocatedTextures[index]->GetDSVDescriptor().CPUHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetRTV(RenderHandle handle)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 index = TextureIndexFromHandle(handle);
		return m_AllocatedTextures[index]->GetRTVDescriptor(0).CPUHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetUAV(RenderHandle handle, Uint32 index)
	{
		KRS_CORE_ASSERT(TypeFromHandle(handle) == RenderResourceType::Texture, "Not the right type");
		Uint64 dex = TextureIndexFromHandle(handle);
		return m_AllocatedTextures[dex]->GetUAVDescriptor(index).CPUHandle();
	}


}