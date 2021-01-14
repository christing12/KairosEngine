	#pragma once

#include <RenderBackend/DescriptorHeap.h>

namespace Kairos {
	class RenderDevice;
	class CommandContext;

	class EngineIMGUI {
	public:
		EngineIMGUI() = default;
		EngineIMGUI(RenderDevice* pDevice, HWND hWnd);

		void Init();
		void Render(class GraphicsContext& context);
		void Shutdown();
	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_IMGUIHeap;
		RenderDevice* m_Device;
		HWND m_HWND;
		DescriptorAllocation m_Allocation;

		void InitColors();
	};
}