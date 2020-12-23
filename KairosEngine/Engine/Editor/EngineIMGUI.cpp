#include "krspch.h"
#include "EngineIMGUI.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "Graphics/Core/RenderDevice.h"
#include "Graphics/Core/CommandContext.h"

namespace Kairos {
	EngineIMGUI::EngineIMGUI(RenderDevice* pDevice, HWND hWnd)
		: m_Device(pDevice)
		, m_HWND(hWnd)
	{
		Init();
	}
	void EngineIMGUI::Init()
	{
		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;

		ImGui::StyleColorsDark();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1024;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		auto hr = m_Device->GetD3DDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_IMGUIHeap));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "ISSUE WITH IMGUI HEAP");

		ImGui_ImplWin32_Init(m_HWND);
		ImGui_ImplDX12_Init(m_Device->GetD3DDevice(), 3, DXGI_FORMAT_R8G8B8A8_UNORM, m_IMGUIHeap.Get(), m_IMGUIHeap->GetCPUDescriptorHandleForHeapStart(),
			m_IMGUIHeap->GetGPUDescriptorHandleForHeapStart());
		
	}

	void EngineIMGUI::Render(GraphicsContext& context)
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool showDemo = true;
		if (showDemo)
			ImGui::ShowDemoWindow(&showDemo);

		ImGui::Render();

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, m_IMGUIHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());

	}
	void EngineIMGUI::Shutdown()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}