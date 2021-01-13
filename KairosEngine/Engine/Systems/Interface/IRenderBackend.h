#pragma once

#include "Core/EngineCore.h"
#include "Core/ClassTemplates.h"

#include "RenderBackend/GraphicsTypes.h"

#include "ISystem.h"

KRS_BEGIN_NAMESPACE(Kairos)

struct IRenderBackendConfig : public ISystemConfig {
	Uint32 NumFrames;
};

class IRenderBackend : public ISystem {
public:
	KRS_INTERFACE_NON_COPYABLE(IRenderBackend);

	virtual Ref<class Texture> CreateTexture(Uint32 width, Uint32 height, Uint32 depth,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, Uint32 levels = 1,
		const std::wstring& debugName = L"Texture") = 0;



	virtual Ref<class GraphicsPSO> AllocateGraphicsPSO(const std::string& name) = 0;
	virtual Ref<class ComputePSO> AllocateComputePSO(const std::string& name) = 0;

	virtual Ref<class RootSignature> AllocateRootSignature(const std::string& name, Uint32 numParams, Uint32 numSamplers = 0) = 0;

	virtual Ref<class PipelineState> GetPipeline(const std::string& name) = 0;
	virtual Ref<class RootSignature> GetRootSignature(const std::string& name) = 0;


	virtual void PreparePipeline(Ref<class PipelineState> pipeline, class GraphicsContext& context) = 0;

	
	virtual D3D12_RECT GetScissor() = 0;
	virtual D3D12_VIEWPORT GetViewport() = 0;
	virtual Uint32 CurrBufferIndex() = 0;
	virtual class RenderDevice* GetRenderDevice() = 0;
	virtual Uint32 GetBackBufferCount() = 0;
	virtual std::pair<Uint32, Uint32> GetScreenResolution() = 0;
	virtual class Texture& GetCurrBackBuffer() = 0;
	virtual void Present() = 0;
};

KRS_END_NAMESPACE