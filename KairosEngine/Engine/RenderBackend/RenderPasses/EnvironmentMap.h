#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RenderPass.h"

KRS_BEGIN_NAMESPACE(Kairos)
class RenderDevice;
class Texture;

class EnvironmentMap {
public:
	EnvironmentMap() = default;

	void Init(RenderDevice* pDevice, PipelineStateManager* manager);

	//virtual void Execute(CommandContext& context, ResourceStorage& storage, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0);

private:
	RenderDevice* m_Device;

	Scope<Texture> m_envTexture;
	Scope<Texture> m_specularTexture;
	Scope<Texture> m_irradianceTexture;
	Scope<Texture> m_specularBRDF;
	Texture* m_equirectTex;
public:
	inline Texture* EnvTexture() { return m_envTexture.get(); }
	inline Texture* Equirect() { return m_equirectTex; }
	inline Texture* IrradianceMap() { return m_irradianceTexture.get(); }
	inline Texture* SpecBRDF() { return m_specularBRDF.get(); }

};

KRS_END_NAMESPACE