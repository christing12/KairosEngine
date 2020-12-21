#pragma once

namespace Kairos {
	class PipelineStateObject {
	public:
		PipelineStateObject(class RenderDevice* pDevice);
		PipelineStateObject(D3D12_GRAPHICS_PIPELINE_STATE_DESC);
		~PipelineStateObject() = default;

		ID3D12PipelineState* GetPipelineState() const { return m_dPipelineState.Get(); }
	public:
		void Finalize();

		void SetRootSignature(ID3D12RootSignature* signature);
		void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc);
		void SetBlendState(const D3D12_BLEND_DESC& blendDesc);
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);
		void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layoutDesc);
		void SetSampleMask(Uint32 sampleMask);
		void SetRenderTargets(Uint32 numRTV, const DXGI_FORMAT* formats, DXGI_FORMAT dsvFormat);
		void SetRenderTarget(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);
		void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC depthStencilDesc);

		void SetVertexShader(const D3D12_SHADER_BYTECODE&);
		void SetPixelShader(const D3D12_SHADER_BYTECODE&);

	private:
		class RenderDevice* m_Device;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_dPipelineState;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc;
	};
}