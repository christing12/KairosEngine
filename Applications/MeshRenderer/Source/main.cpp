#pragma once
#include "apppch.h"
#include "MeshRenderer.h"
#include <memory>

using namespace Kairos;

int main() {
	Kairos::Log::Init();

	Application* app = new MeshRenderer();
	app->Create();
	app->InitEngine();
	app->Run();
	app->Shutdown();
	delete app;

#if defined(_DEBUG)
	Microsoft::WRL::ComPtr<IDXGIDebug1> pDebug;
	auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug));
	KRS_CORE_ASSERT(SUCCEEDED(hr), "Issues");
	if (SUCCEEDED(hr)) {
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	}
#endif

	return 0;
}