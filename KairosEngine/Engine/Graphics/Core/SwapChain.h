#pragma once

#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>


KRS_BEGIN_NAMESPACE(Kairos)
class Texture;
class RenderDevice;

class SwapChain {
public:
	SwapChain(const DXGI_SWAP_CHAIN_DESC1& scDesc, Ref<RenderDevice> pDevice, HWND hWnd);
	~SwapChain();

	void Resize(Uint32 newWidth, Uint32 newHeight);


	RenderDevice* GetRenderDevice() { return m_Device.get(); }
	const DXGI_SWAP_CHAIN_DESC1& GetDesc() { return m_Desc; }
	IDXGISwapChain4* GetD3DSwapChain() { return m_dSwapChain.Get(); }

	Ref<Texture> GetBackBuffer(size_t idx) { return m_BackBuffers[idx]; }
	Uint32 CurrBackBuffer() { return m_dSwapChain->GetCurrentBackBufferIndex(); }
	Ref<Texture> GetDepthBuffer() { return m_DepthBuffer; }

	void Present();
protected:
	void CreateDXGISwapChain(HWND hWnd);
	void CreateRTVAndDSV();
private:
	Uint32 m_NumFrames = 0;
	Uint32 m_Width = 0;
	Uint32 m_Height = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_dSwapChain = nullptr;
	Ref<RenderDevice> m_Device = nullptr;
	DXGI_SWAP_CHAIN_DESC1 m_Desc;

	std::vector<Ref<Texture> > m_BackBuffers;
	Ref<Texture> m_DepthBuffer;
};


KRS_END_NAMESPACE