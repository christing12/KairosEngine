#pragma once
#include "apppch.h"
#include "App.h"


#include <memory>

using namespace Kairos;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int nCmdshow) {
	AllocConsole();

	errno_t err;
	FILE* stream;
	err = freopen_s(&stream, "CONOUT$", "w", stdout);

	Kairos::Log::Init();

	SetConsoleTitle(L"Kairos Engine Console");
	Scope<ApplicationEntry> app = CreateScope<App>();

	if (!app->Setup(hInstance, nullptr, nullptr)) {
		KRS_ERROR("CAN't Setup App");
		return 0;
	}

	if (!app->Initialize()) {
		KRS_ERROR("Can't init app");
		return 0;
	}

	app->Run();
	app->Shutdown();
	
//#if defined(_DEBUG)
//	Microsoft::WRL::ComPtr<IDXGIDebug1> pDebug;
//	auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug));
//	KRS_CORE_ASSERT(SUCCEEDED(hr), "Issues");
//	if (SUCCEEDED(hr)) {
//		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
//	}
//#endif
	app.reset();

	return 0;


}