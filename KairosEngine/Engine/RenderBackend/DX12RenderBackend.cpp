#include "krspch.h"
#include "DX12RenderBackend.h"

#include "RHI/RenderDevice.h"
#include "RHI/SwapChain.h"
#include "Systems/Interface/IEngine.h"
#include "Scene/Scene.h"

extern Kairos::IEngine* g_Engine;

namespace Kairos {

   

    bool DX12RenderBackend::CreateDebugLayer() {
       // ComPtr<ID3D12Debug> debugInterface;

        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface));
        if (FAILED(hr)) {
            KRS_CORE_ERROR("Render Backend failed to get a debug interface");
            return false;
        }

        //hr = debugInterface.As(&m_debugInterface);
        //if (FAILED(hr)) {
        //    KRS_CORE_ERROR("Failed to Query ID3D12Debug1 Interface for the Render Backend");
        //    return false;
        //}

        m_debugInterface->EnableDebugLayer();


        return true;
    }

    bool DX12RenderBackend::CreateRenderDevice() {

        UINT dxgiFlag = 0;
#if defined(_DEBUG)
        dxgiFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
        ComPtr<IDXGIFactory2> dxgiFactory;
        auto hr = CreateDXGIFactory2(dxgiFlag, IID_PPV_ARGS(&dxgiFactory));
        
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating DXGI Factory");
        

        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        ComPtr<IDXGIAdapter4> dxgiAdapter4;
        SIZE_T maxDedicatedVideoMemory = 0;

        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
            DXGI_ADAPTER_DESC1 adapterDesc;
            dxgiAdapter1->GetDesc1(&adapterDesc);

            if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
                continue;

            auto hr = D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr);
            if (SUCCEEDED(hr) && adapterDesc.DedicatedVideoMemory > maxDedicatedVideoMemory) {
                maxDedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
                dxgiAdapter1.As(&dxgiAdapter4);
            }
        }

        if (dxgiAdapter4 != nullptr) {
            KRS_CORE_INFO("Correct GPU Adapter found!");
            m_Adapter = dxgiAdapter4;
        }

        // enumerates adapter output (the monitor!)
        ComPtr<IDXGIOutput> adapterOutput;
        hr = m_Adapter->EnumOutputs(0, &adapterOutput);
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create monitor adapter?");

        hr = adapterOutput.As(&m_AdapterOutput);
        KRS_CORE_ASSERT(SUCCEEDED(hr), "??????????????????");


        Uint32 numModes;
        Uint64 stringLen;

        hr = m_AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
        KRS_CORE_ASSERT(SUCCEEDED(hr), "no dispalys support right DXGI FORMAT");


        //std::vector<DXGI_MODE_DESC> displayModeList(numModes);
        //hr = m_AdapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8_UNORM,
        //    DXGI_ENUM_MODES_INTERLACED, &numModes, &displayModeList[0]);


        m_Adapter->GetDesc(&m_AdapterDesc);

        m_VideoCardMemory = (int32_t)(m_AdapterDesc.DedicatedVideoMemory / 1024 / 1024);
        if (wcstombs_s(&stringLen, m_VideoCardDescription, 128, m_AdapterDesc.Description, 128) != 0) {

        }


        ComPtr<ID3D12Device5> pdDevice;
        hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pdDevice));
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with Creating ID3D12Device");




        ComPtr<ID3D12InfoQueue> pInfoQueue;
        KRS_CORE_ASSERT(SUCCEEDED(pdDevice.As(&pInfoQueue)), "Error with creatin debug info queue");
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
       // pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

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

        pdDevice->SetName(L"Logical Device");
        m_RenderDevice = CreateRef<RenderDevice>(pdDevice);
        return true;


    }
    bool DX12RenderBackend::CreateSwapChain() {

        // --- Creating Swap Chain --- //
        DXGI_SWAP_CHAIN_DESC1 scDesc;
        ZeroMemory(&scDesc, sizeof(scDesc));
        scDesc.Width = WINDOW_WIDTH;                             // resolution width
        scDesc.Height = WINDOW_HEIGHT;                             // resolution height
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;       // display format (32 bit 8 bit r, g, b, a)
        scDesc.Stereo = FALSE;
        scDesc.SampleDesc = { 1, 0 };                         // must be 1, 0
        scDesc.BufferCount = m_SwapChainCount;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.Scaling = DXGI_SCALING_STRETCH;
        scDesc.Flags = 0;


        m_SwapChain = CreateRef<SwapChain>(m_RenderDevice.get(), scDesc);


        m_Scissor = CD3DX12_RECT(0, 0, scDesc.Width, scDesc.Height);
        m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, scDesc.Width, scDesc.Height, 0.f, 1.f);

        return true;
    }


	bool DX12RenderBackend::Setup(ISystemConfig* config) {
        IRenderBackendConfig* con = reinterpret_cast<IRenderBackendConfig*>(config);
        m_SwapChainCount = con->NumFrames;


#if defined(_DEBUG)
        bool result = CreateDebugLayer();
#endif

        CreateRenderDevice();
        CreateSwapChain();

        return true;
	}

	bool DX12RenderBackend::Init()
	{

        std::string path = "Data/shaders/";
        KRS_CORE_INFO(Filesystem::PathExists(path));


		return true;
	}

	bool DX12RenderBackend::Update()
	{
		return true;
	}

	bool DX12RenderBackend::Shutdown()
	{
        m_RenderDevice->Flush();
        m_RenderDevice->Shutdown();
        m_SwapChain.reset();
       // m_RenderDevice.reset();
        m_Adapter = nullptr;
        m_AdapterOutput = nullptr;
        m_debugInterface = nullptr;

		return true;
	}

    RenderDevice* DX12RenderBackend::GetRenderDevice()
    {
        KRS_CORE_ASSERT(m_RenderDevice != nullptr, "IEWJFOWPIFEJ");
        return m_RenderDevice.get();
    }

    void DX12RenderBackend::Present()
    {
        m_RenderDevice->Present(m_SwapChain.get());
    }

    void DX12RenderBackend::BeginFrame()
    {
    }

    void DX12RenderBackend::RenderScene(Scene& scene)
    {
        
    }

    Texture* DX12RenderBackend::GetCurrBackBuffer()
    {
        return m_SwapChain->GetBackBuffer(m_SwapChain->CurrBackBuffer());
    }

    Texture* DX12RenderBackend::GetDepthBuffer()
    {
        return m_SwapChain->GetDepthBuffer();
    }

    Uint32 DX12RenderBackend::GetCurrIndex() const
    {
        return m_SwapChain->CurrBackBuffer();
    }

}

