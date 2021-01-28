#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RenderPass.h"

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class Texture;

struct GenerateMipsConstants {
	Uint32 inputTextureIndex;
	Uint32 outputTextureIndex;
};

class GenerateMips {
public:
	GenerateMips() = default;

	void Init(RenderDevice* pDevice, PipelineStateManager* manager);

	void Generate(Texture* inputTexture);

	//virtual void Execute(CommandContext& context, ResourceStorage& storage, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0);
private:
	RenderDevice* m_Device;
};

KRS_END_NAMESPACE