#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

KRS_BEGIN_NAMESPACE(Kairos)

class Texture;
class RenderDevice;

struct Material
{
	Material() = default;

	static Material LoadMatFromFolder(RenderDevice* pDevice, const std::string& folderPath, const std::string& matName);

	Texture* Albedo;
	Texture* NormalMap;
	Texture* Metalness;
	Texture* Roughness;
	Texture* AOMap;

	std::string albedoFile;
	std::string normalFile;
	std::string metalFile;
	std::string roughFile;
	std::string AOMapFile;
};



KRS_END_NAMESPACE