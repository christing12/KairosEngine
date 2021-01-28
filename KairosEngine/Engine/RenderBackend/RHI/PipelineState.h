#pragma once
#include "GraphicsTypes.h"

namespace Kairos {
	class RootSignature;
	class RenderDevice;

	struct Shader;

	class PipelineState {
	public:
		PipelineState(class RenderDevice* pDevice);
		virtual ~PipelineState() = default;

		ID3D12PipelineState* GetPipelineState() const { return m_dPipelineState.Get(); }
		virtual void Finalize() {}
		void SetDebugName(const std::string& name);

		class RootSignature* GetSignature() { return m_RootSig; }

	protected:
		RenderDevice* m_Device = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_dPipelineState = nullptr;
		class RootSignature* m_RootSig;
		std::string m_DebugName = "";
		bool isCompiled = false;
	};

	class GraphicsPSO : public PipelineState {
	public:
		GraphicsPSO(class RenderDevice* pDevice);

		void Finalize();

		void SetRootSignature(RootSignature* signature);
		void SetVertexShader(Shader* vertexShader);
		void SetPixelShader(Shader* pixelShader);
		void SetHullShader(Shader* hullShader);
		void SetDomainShader(Shader* domainShader);
		void SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layoutDesc);
		void SetRenderTargets(Uint32 numRTV, const DXGI_FORMAT* formats, DXGI_FORMAT dsvFormat);
		void SetRenderTarget(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat);

		void SetBlendState(const D3D12_BLEND_DESC& blendDesc);
		void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc);
		void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC depthStencilDesc);
		void SetSampleMask(Uint32 sampleMask);
		void SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology);

	private:
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_Desc;

	public:
		inline const D3D12_GRAPHICS_PIPELINE_STATE_DESC& Desc() const { return m_Desc; }

	};

	class ComputePSO : public PipelineState {
	public:
		ComputePSO(class RenderDevice* pDevice);

		void Finalize();

		void SetRootSignature(RootSignature* signature);
		void SetComputeShader(Shader* computeShader);
	private:
		D3D12_COMPUTE_PIPELINE_STATE_DESC m_Desc;
	};






















}