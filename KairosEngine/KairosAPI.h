#pragma once

#include "Graphics/Core/GraphicsTypes.h"

#include "Engine/Core/EngineCore.h"
#include "Engine/Core/BaseTypes.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/EngineMath.h"
#include "Engine/Core/Log.h"

namespace Kairos {
    extern Application* CreateApplication();
}



// -- GRAPHICS --- //
#include "Graphics/Core/Buffer.h"
#include "Graphics/Core/PipelineStateObject.h"
#include "Graphics/Core/RootSignature.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Mesh.h"
#include "Graphics/Core/Renderer.h"
#include "Graphics/Core/Shader.h"


int main()
{
    Kairos::Application* app = Kairos::CreateApplication();
    app->Create();
    app->InitEngine();
    app->Run();
    app->Shutdown();

    delete app;
    //#if defined(_DEBUG)
    //    ComPtr<IDXGIDebug1> pDebug;
    //    auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug));
    //    KRS_CORE_ASSERT(SUCCEEDED(hr), "Issues");
    //    if (SUCCEEDED(hr)) {
    //        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    //    }
    //#endif
    return 0;
}

