#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include "RenderBackend/RenderPass.h"
#include "Scene/Mesh.h"

namespace dxmath = DirectX::SimpleMath;
KRS_BEGIN_NAMESPACE(Kairos)

class Buffer;
class Mesh;
class DynamicBuffer;

struct SkyboxConstants
{
	dxmath::Matrix viewProjMat;
	Uint32 skyboxTextureIndex;
};
class RenderDevice;

class SkyboxPass : public RenderPass {
public:
	SkyboxPass();

	void WriteData(SkyboxConstants& constants);

	virtual void Setup(RenderDevice* pDevice, PipelineStateManager* manager);

	virtual void Execute(CommandContext& context, ResourceStorage& storage, CPVoid constantBufferData = nullptr, Uint32 dataSize = 0);
private:
	Mesh m_skyboxMesh;
	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	Scope<DynamicBuffer> m_ConstBuffer;
};

KRS_END_NAMESPACE