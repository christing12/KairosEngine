#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>


KRS_BEGIN_NAMESPACE(Kairos)

class PipelineState;
class RootSignature;
class ComputePSO;

class MipMapGenerator {
public:
	MipMapGenerator() = default;
	MipMapGenerator(class RenderDevice* pDevice);

	void GenerateMipMap(const class Texture& texture);
private:
	class RenderDevice* m_Device;

	Ref<ComputePSO> m_GammaPipeline;
	Ref<ComputePSO> m_ArrayPipeline;
	Ref<ComputePSO> m_LinearPipeline;
	Ref<RootSignature> m_Signature;
};


KRS_END_NAMESPACE