#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RenderPass.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct TonemapConstants
{

};

class TonemapPass : public RenderPass {
public:
	TonemapPass();

	virtual void Setup(RenderDevice* pDevice, PipelineStateManager* manager);

	virtual void Execute(CommandContext& context, ResourceStorage& storage, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0);
};

KRS_END_NAMESPACE