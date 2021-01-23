#pragma once

#include "Systems/Interface/IRenderBackend.h"


// Provides Video Memory Query, Adapter Support, Render Device, Render Graph implementation for D3D12
// RenderDoc & PIX integration validation layers 

KRS_BEGIN_NAMESPACE(Kairos)


class Scene;


class DX12RenderBackend {
public:

	KRS_INTERFACE_NON_COPYABLE(DX12RenderBackend);
	bool Setup(ISystemConfig* config);
	bool Init();
	bool Update();
	bool Shutdown();
	class RenderDevice* GetRenderDevice();
	void Present();
    void BeginFrame();
    void RenderScene(Scene& scene);
    class Texture* GetCurrBackBuffer();
    class Texture* GetDepthBuffer();
    Uint32 GetCurrIndex() const;

private:
    Ref<class RenderDevice> m_RenderDevice;
    Ref<class SwapChain> m_SwapChain;

    Uint32 m_CurrBackBuffer;
    D3D12_RECT m_Scissor;
    D3D12_VIEWPORT m_Viewport;

    Uint32 m_SwapChainCount = 2;

    bool CreateDebugLayer();
    bool CreateRenderDevice();
    bool CreateSwapChain();

    Microsoft::WRL::ComPtr<ID3D12Debug> m_debugInterface;
    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_Adapter;
    Microsoft::WRL::ComPtr<IDXGIOutput1> m_AdapterOutput;
    DXGI_ADAPTER_DESC m_AdapterDesc;
    int32_t m_VideoCardMemory = 0;
    char m_VideoCardDescription[128];
};


KRS_END_NAMESPACE