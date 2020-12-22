#pragma once


#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"

#define SWAP_CHAIN_BUFFER_COUNT 2

namespace Kairos {
	class RenderDevice;
	class CommandQueue;
	class SwapChain;
	class CommandContext;
	class ColorBuffer;

	class Renderer {
	public:
		//static void Setup();
		//static void Shutdown();

		//static void BeginContext();
		//static void SubmitContext();

		//static void CreateShader();
		//static void CreateTexture();
		//static void CreateBuffer();
		//static void CreatePipeline();
	public:
		Renderer() = default;
		Renderer(HWND hWnd);
		~Renderer();
		class GraphicsContext& GetGraphicsContext();

		class Texture& GetCurrBackBuffer();
		D3D12_CPU_DESCRIPTOR_HANDLE GetDSV();
		class Texture& GetDepthBuffer();

		ID3D12Device2* GetDevice();
		RenderDevice* GetRenderDevice() { return m_Device.get(); }

		void Initialize();
		void SubmitCommandContext(Ref<class CommandContext> commandContext, bool waitForCompletion);
		void Shutdown();

		void Render();
		void Present();


		HWND m_HWND;

		Ref<RenderDevice> m_Device;
		Ref<SwapChain> m_SwapChain;
		size_t m_currBackBuffer = 0;

		D3D12_RECT m_Scissor;
		D3D12_VIEWPORT m_Viewport;


		D3D12_RASTERIZER_DESC DefaultRasterizer;
	private:

	};
}