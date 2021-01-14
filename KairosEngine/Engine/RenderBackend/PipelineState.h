#pragma once


#include "GraphicsTypes.h"

namespace Kairos {
	class RootSignature;

	struct Shader;





















	class PipelineState {
	public:
		PipelineState(class RenderDevice* pDevice);
		~PipelineState() = default;

		ID3D12PipelineState* GetPipelineState() const { return m_dPipelineState.Get(); }

		void SetDebugName(const std::string& name);
	protected:
		class RenderDevice* m_Device = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_dPipelineState = nullptr;
		std::string m_DebugName = "";
	};

	class GraphicsPSO : public PipelineState {
	public:
		GraphicsPSO(class RenderDevice* pDevice);

		void Finalize();

		void SetRootSignature(const RootSignature* signature);
		void SetVertexShader(Shader* vertexShader);
		void SetPixelShader(Shader* pixelShader);
		void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layoutDesc);
		void SetRenderTargets(Uint32 numRTV, const DXGI_FORMAT* formats, DXGI_FORMAT dsvFormat);
		void SetRenderTarget(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);

		void SetBlendState(const D3D12_BLEND_DESC& blendDesc);
		void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc);
		void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC depthStencilDesc);
		void SetSampleMask(Uint32 sampleMask);
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);


		void InitializeCommonFeatures(
			const D3D12_BLEND_DESC& blendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
			const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT),
			const D3D12_RASTERIZER_DESC& rasterDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT),
			Uint32 sampleMask = 0xffffffff,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
		);

		D3D12_RECT Scissor() const { return m_Scissor; }
		D3D12_VIEWPORT Viewport() const { return m_Viewport; }

	private:
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc;
		D3D12_RECT m_Scissor;
		D3D12_VIEWPORT m_Viewport;

	};

	class ComputePSO : public PipelineState {
	public:
		ComputePSO(class RenderDevice* pDevice);

		void Finalize();

		void SetRootSignature(RootSignature& signature);
		void SetComputeShader(Shader* computeShader);
	private:
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_Desc;
	};






















}