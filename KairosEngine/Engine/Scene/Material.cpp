#include "krspch.h"
#include "Material.h"
#include "RenderBackend/RHI/RenderDevice.h"


namespace Kairos {
	Material Material::LoadMatFromFolder(RenderDevice* pDevice, const std::string& folderPath, const std::string& matName)
	{
		Material mat;
		mat.albedoFile = folderPath + matName + "_A.tx";
		mat.normalFile = folderPath + matName + "_N.tx";
		mat.metalFile = folderPath + matName + "_M.tx";
		mat.roughFile = folderPath + matName + "_R.tx";
		mat.AOMapFile = folderPath + matName + "_AO.tx";

		mat.Albedo		= pDevice->CreateTexture(mat.albedoFile.c_str(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		mat.NormalMap	= pDevice->CreateTexture(mat.normalFile.c_str(), DXGI_FORMAT_R8G8B8A8_UNORM);
		mat.Metalness	= pDevice->CreateTexture(mat.metalFile.c_str(), DXGI_FORMAT_R8_UNORM);
		mat.Roughness	= pDevice->CreateTexture(mat.roughFile.c_str(), DXGI_FORMAT_R8_UNORM);

		return mat;
	}

}

