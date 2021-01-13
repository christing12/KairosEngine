#pragma once

#include "Systems/Interface/IRenderBackend.h"


// Provides Video Memory Query, Adapter Support, Render Device, Render Graph implementation for D3D12
// RenderDoc & PIX integration validation layers 

KRS_BEGIN_NAMESPACE(Kairos)

class DX12RenderBackend {
public:

	KRS_INTERFACE_NON_COPYABLE(DX12RenderBackend);
	bool Setup(ISystemConfig* config);
	bool Init();
	bool Update();
	bool Shutdown();


	Ref<class GraphicsPSO> AllocateGraphicsPSO(const std::string& name);
	Ref<class ComputePSO> AllocateComputePSO(const std::string& name);

	Ref<class RootSignature> AllocateRootSignature(const std::string& name, Uint32 numParams, Uint32 numSamplers = 0);

	Ref<class PipelineState> GetPipeline(const std::string& name);
	Ref<class RootSignature> GetRootSignature(const std::string& name);
	void PreparePipeline(Ref<class PipelineState> pipeline, class GraphicsContext& context);


public:

	D3D12_RECT GetScissor();
	D3D12_VIEWPORT GetViewport();
	Ref<class Texture> CreateTexture(Uint32 width, Uint32 height, Uint32 depth,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, Uint32 levels = 1,
		const std::wstring& debugName = L"Texture");
	class RenderDevice* GetRenderDevice();
	Uint32 GetBackBufferCount();
	std::pair<Uint32, Uint32> GetScreenResolution();
	class Texture& GetCurrBackBuffer() ;
	Uint32 CurrBufferIndex();

	void Present();
};


KRS_END_NAMESPACE