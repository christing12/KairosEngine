#pragma once

#include <wrl.h>

namespace Kairos {
	struct EngineCreateInfo {
		bool Debug = false;
	};
	typedef struct EngineCreateInfo EngineCreateInfo;

	class EngineFactory {
	public:
		static EngineFactory* Instance() {
			static EngineFactory factory;
			return &factory;
		}

		void CreateRenderDevice(Ref<class RenderDevice>& pDevice, const EngineCreateInfo& createInfo);
		void CreateSwapChain(Ref<class RenderDevice> pDevice, const DXGI_SWAP_CHAIN_DESC1& scDesc, Ref<class SwapChain>& pSwapChain, HWND hWnd);
	private:
		EngineFactory() {}
		~EngineFactory() = default;

		void GetAdapter(IDXGIFactory4* pFactory, IDXGIAdapter4** ppAdapter);
		void CreateDeviceDebug(Microsoft::WRL::ComPtr<ID3D12Device2> pdDevice);
	};
}