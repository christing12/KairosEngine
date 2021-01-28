#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>


KRS_BEGIN_NAMESPACE(Kairos)

KRS_BEGIN_NAMESPACE(DX12Util)

D3D12_SHADER_RESOURCE_VIEW_DESC GetSRVDesc(const D3D12_RESOURCE_DESC& desc, Uint32 mostDetailedMip = 0)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (desc.DepthOrArraySize) {
	case 1:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = mostDetailedMip;
		break;
	case 6:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MostDetailedMip = mostDetailedMip;
		srvDesc.TextureCube.MipLevels = desc.MipLevels;
		break;
	default:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = mostDetailedMip;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
		break;
	}
	return srvDesc;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const D3D12_RESOURCE_DESC& desc, Uint32 mipSlice, Uint32 arraySlice = 0, Uint32 planeSlice = 0)
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = desc.Format;
	switch (desc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (desc.DepthOrArraySize > 1) {
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize - arraySlice;
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


KRS_END_NAMESPACE


KRS_END_NAMESPACE