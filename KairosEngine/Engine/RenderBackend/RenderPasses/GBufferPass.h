#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RenderPass.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct GBufferPassConstants
{
	Uint32 specularIndex;
	Uint32 irradianceIndex;
	Uint32 lutIndex;
};

class GBufferPass : public RenderPass {
public:
	GBufferPass();
	~GBufferPass();
	void WriteData(GBufferPassConstants& constants);
	virtual void Setup(RenderDevice* pDevice, Kairos::PipelineStateManager* manager) override;
	virtual void Execute(Kairos::CommandContext& context, Kairos::ResourceStorage& storage, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0) override;
private:
	Scope<class DynamicBuffer> m_ConstBuffer;
};

KRS_END_NAMESPACE