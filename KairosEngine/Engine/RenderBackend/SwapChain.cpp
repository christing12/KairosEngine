#include "krspch.h"
#include "SwapChain.h"
#include "DescriptorHeap.h"
#include "CommandQueue.h"
#include "RenderDevice.h"
#include "Resource.h"
#include "Texture.h"


#include "Systems/Interface/IEngine.h"
#include "Systems/WinWindowSystem.h"

extern Kairos::IEngine* g_Engine;

namespace Kairos {

	SwapChain::SwapChain(RenderDevice* pDevice, const DXGI_SWAP_CHAIN_DESC1& scDesc)
		: m_Desc(scDesc)
		, m_Device(pDevice)
		, m_NumFrames(scDesc.BufferCount)
		, m_Width(scDesc.Width)
		, m_Height(scDesc.Height)
	{
		CreateDXGISwapChain(reinterpret_cast<WinWindowSystem*>(g_Engine->GetWindowSystem())->GetHWND());
		CreateRTVAndDSV();
	}

	SwapChain::~SwapChain() {
		m_dSwapChain = nullptr;
		m_BackBuffers.clear();
	}

	void SwapChain::Resize(Uint32 newWidth, Uint32 newHeight)
	{
		m_Width = newWidth;
		m_Height = newHeight;

		m_Device->Flush();
		m_BackBuffers.clear();

		for (UINT frame = 0; frame < m_BackBuffers.size(); frame++) {

		}

		if (m_dSwapChain != nullptr) {
			auto hr = m_dSwapChain->ResizeBuffers(m_NumFrames, m_Width, m_Height, m_Desc.Format, 0);
			KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to resize swap chain");
		}


	}

	void SwapChain::CreateDXGISwapChain(HWND hwnd)
	{
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		auto result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		KRS_CORE_ASSERT(SUCCEEDED(result), "Issue with creating idxgifactory for swap chain");

		ComPtr<IDXGISwapChain1> swapChainTemp;
		result = dxgiFactory->CreateSwapChainForHwnd(
			m_Device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetD3DCommandQueue(),
			hwnd,
			&m_Desc,
			nullptr,								// optional for full screen swap chain
			nullptr,
			swapChainTemp.ReleaseAndGetAddressOf()
		);
		KRS_CORE_ASSERT(SUCCEEDED(result), "Issue with creating swap chain");

		result = dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER); // disalbe alt + enter
		KRS_CORE_ASSERT(SUCCEEDED(result), "Issuew ith disabling Alt Enter");

		result = swapChainTemp.As(&m_dSwapChain);
		KRS_CORE_ASSERT(SUCCEEDED(result), "Issue with convert from SwapChain1 to SwapChain4");
	}


	void SwapChain::CreateRTVAndDSV()
	{
		m_BackBuffers.resize(m_Desc.BufferCount);
		HRESULT hr;
		for (Uint32 frame = 0; frame < m_Desc.BufferCount; ++frame) {
			ComPtr<ID3D12Resource> backBuffer;
			hr = m_dSwapChain->GetBuffer(frame, IID_PPV_ARGS(&backBuffer));
			KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with getting back buffer from swap chian");

			RenderHandle handle = m_Device->CreateTexture(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
			m_BackBuffers[frame] = m_Device->m_TextureManager.GetUnreferencedTexture(handle);
		}
		D3D12_CLEAR_VALUE clearVal = {};
		clearVal.Format = DXGI_FORMAT_D32_FLOAT;
		clearVal.DepthStencil = { 1.0f, 0 };

		D3D12_RESOURCE_DESC depthBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
			m_Width, m_Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		depthBufferDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		RenderHandle handle = m_Device->CreateTexture(depthBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearVal);
		m_DepthBuffer = m_Device->m_TextureManager.GetUnreferencedTexture(handle);
	}

	void SwapChain::Present() {
		auto hr = m_dSwapChain->Present(1, 0);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with Preseting Swap Chain");
	}
}