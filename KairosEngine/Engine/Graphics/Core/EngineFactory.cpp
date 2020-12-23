#include "krspch.h"
#include "EngineFactory.h"
#include "RenderDevice.h"
#include "CommandQueue.h"
#include "SwapChain.h"

#pragma comment (lib, "d3d11.lib") 
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace Kairos {
	void EngineFactory::CreateRenderDevice(Ref<RenderDevice>& pDevice, const EngineCreateInfo& createInfo)
	{
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;

		if (createInfo.Debug) {
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}

		auto result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
		KRS_CORE_ASSERT(SUCCEEDED(result), "Issue with creating DXGI Factory");

		ComPtr<IDXGIAdapter4> dxgiAdapter4; // the rightversion
		GetAdapter(dxgiFactory.Get(), dxgiAdapter4.ReleaseAndGetAddressOf());


		ComPtr<ID3D12Device2> pdDevice; // for windows 10 creator update
		result = D3D12CreateDevice(dxgiAdapter4.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pdDevice));
		KRS_CORE_ASSERT(SUCCEEDED(result), "Issue with creating the ID3D12Device");


		if (createInfo.Debug) {
			this->CreateDeviceDebug(pdDevice);
		}
		pDevice = CreateRef<RenderDevice>(pdDevice.Get());
		pDevice->CreateCommandQueues();
	}

	void EngineFactory::CreateSwapChain(Ref<RenderDevice> pDevice, const DXGI_SWAP_CHAIN_DESC1& scDesc, Ref<SwapChain>& pSwapChain, Window& window)
	{
		pSwapChain = CreateRef<SwapChain>(scDesc, pDevice, window);
	}


	void EngineFactory::GetAdapter(IDXGIFactory4* pFactory, IDXGIAdapter4** ppAdapter)
	{
		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;
		SIZE_T maxDedicatedVideoMemory = 0;

		for (UINT i = 0; pFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 adapterDesc;
			dxgiAdapter1->GetDesc1(&adapterDesc);

			if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
					D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
				adapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
				KRS_CORE_ASSERT(SUCCEEDED(dxgiAdapter1.As(&dxgiAdapter4)), "error with this");
			}
		}

	}
	void EngineFactory::CreateDeviceDebug(ComPtr<ID3D12Device2> pdDevice)
	{
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		KRS_CORE_ASSERT(SUCCEEDED(pdDevice.As(&pInfoQueue)), "Error with creatin debug info queue");
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		NewFilter.DenyList.NumSeverities = _countof(severities);
		NewFilter.DenyList.pSeverityList = severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		KRS_CORE_ASSERT(SUCCEEDED(pInfoQueue->PushStorageFilter(&NewFilter)), "error with storage filter");
	}
}