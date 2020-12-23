#include "krspch.h"
#include "Renderer.h"
#include "EngineFactory.h"
#include "CommandQueue.h"
#include "CommandContext.h"
#include "RenderDevice.h"
#include "SwapChain.h"
#include "Texture.h"
#include "Core/Window.h"

namespace Kairos {
    const Uint32 NUM_FRAMES = 2; // replace this somewhere else

    Renderer::~Renderer()
    {
        m_Device->Shutdown();
    }

    GraphicsContext& Renderer::GetGraphicsContext()
    {
        CommandContext& test = m_Device->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
        return test.GetGraphicsContext();
    }

    Texture& Renderer::GetCurrBackBuffer()
    {
        return *(m_SwapChain->GetBackBuffer(m_currBackBuffer));
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Renderer::GetDSV()
    {
        return m_SwapChain->GetDepthBuffer()->GetDSV();
    }

    Texture& Renderer::GetDepthBuffer()
    {
        return *(m_SwapChain->GetDepthBuffer());
    }

    ID3D12Device2* Renderer::GetDevice()
    {
        return m_Device->GetD3DDevice();
    }

    void Renderer::Initialize(Window& window)
    {
#if defined(_DEBUG)
        ComPtr<ID3D12Debug> debugInterface;
        auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
        KRS_ASSERT(SUCCEEDED(hr), "Issue creating debug Interface");
        debugInterface->EnableDebugLayer();
#endif


        EngineFactory* eFactory = EngineFactory::Instance();
        eFactory->CreateRenderDevice(m_Device, EngineCreateInfo{ false });

        m_Device->GetD3DDevice()->SetName(L"LMFAO");


        // --- Creating Swap Chain --- //
        DXGI_SWAP_CHAIN_DESC1 scDesc;
        ZeroMemory(&scDesc, sizeof(scDesc));
        scDesc.Width = WINDOW_WIDTH;                             // resolution width
        scDesc.Height = WINDOW_HEIGHT;                             // resolution height
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;       // display format (32 bit 8 bit r, g, b, a)
        scDesc.Stereo = FALSE;
        scDesc.SampleDesc = { 1, 0 };                         // must be 1, 0
        scDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.Scaling = DXGI_SCALING_STRETCH;
        scDesc.Flags = 0;

        eFactory->CreateSwapChain(m_Device, scDesc, m_SwapChain, window);

        // Default rasterizer states
        DefaultRasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        DefaultRasterizer.CullMode = D3D12_CULL_MODE_BACK;
        DefaultRasterizer.FrontCounterClockwise = FALSE;
        DefaultRasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        DefaultRasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        DefaultRasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        DefaultRasterizer.DepthClipEnable = TRUE;
        DefaultRasterizer.MultisampleEnable = FALSE;
        DefaultRasterizer.AntialiasedLineEnable = FALSE;
        DefaultRasterizer.ForcedSampleCount = 0;
        DefaultRasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        m_Scissor = CD3DX12_RECT(0, 0, scDesc.Width, scDesc.Height);
        m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, scDesc.Width, scDesc.Height, 0.f, 1.f);
    }


    void Renderer::SubmitCommandContext(Ref<class CommandContext> commandContext, bool waitForCompletion)
    {
        Uint64 fenceValue = commandContext->Submit(waitForCompletion);
    }
    void Renderer::Shutdown()
    {
        // m_Device->Flush();
    }
    void Renderer::Render()
    {
        GraphicsContext& gfxContext = GetGraphicsContext();
        auto& currBackBuffer = GetCurrBackBuffer();

        gfxContext.TransitionResource(currBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
        gfxContext.ClearColor(currBackBuffer);
        gfxContext.Submit(true);
    }
    void Renderer::Present()
    {
        GraphicsContext& gfxContext = GetGraphicsContext();
        gfxContext.TransitionResource(GetCurrBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, true);
        gfxContext.Submit(true);

        m_SwapChain->Present();
        m_currBackBuffer = m_SwapChain->GetD3DSwapChain()->GetCurrentBackBufferIndex();
        m_Device->ReleaseStaleDescriptors();
    }
}