#include "krspch.h"
#include "PipelineStateObject.h"
#include "RootSignature.h"
#include "RenderDevice.h"

namespace Kairos {

	PipelineStateObject::PipelineStateObject(RenderDevice* pDevice)
		: m_Device(pDevice)
	{
		ZeroMemory(&m_Desc, sizeof(m_Desc));
		m_Desc.NodeMask = 1;
		m_Desc.SampleMask = 0xFFFFFFFFu;
		m_Desc.SampleDesc.Count = 1;
		m_Desc.InputLayout.NumElements = 0;

		//auto hr = m_Device->GetD3DDevice()->CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&m_dPipelineState));
		//KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating graphics piplien state object");
	}

	PipelineStateObject::PipelineStateObject(D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc)
		: m_Desc(psoDesc)
	{
	}
	void PipelineStateObject::Finalize()
	{
		auto hr = m_Device->GetD3DDevice()->CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&m_dPipelineState));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with finalizing pipeline state");
	}
	//void PipelineStateObject::SetRootSignature(RootSignature* rootSignature)
	//{
	//	m_Desc.pRootSignature = rootSignature->GetD3DRootSignature();
	//}
	void PipelineStateObject::SetVertexShader(const D3D12_SHADER_BYTECODE& vertexBytecode)
	{
		m_Desc.VS = vertexBytecode;
	}
	void PipelineStateObject::SetPixelShader(const D3D12_SHADER_BYTECODE& pixelBytecode)
	{
		m_Desc.PS = pixelBytecode;
	}
	void PipelineStateObject::SetRootSignature(ID3D12RootSignature* signature)
	{
		m_Desc.pRootSignature = signature;
	}
	void PipelineStateObject::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc)
	{
		m_Desc.RasterizerState = rasterDesc;
	}
	void PipelineStateObject::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
	{
		m_Desc.BlendState = blendDesc;
	}
	void PipelineStateObject::SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
	{
		m_Desc.PrimitiveTopologyType = topology;
	}
	void PipelineStateObject::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layoutDesc)
	{
		m_Desc.InputLayout = layoutDesc;

	}
	void PipelineStateObject::SetSampleMask(Uint32 sampleMask)
	{
		DXGI_SAMPLE_DESC desc = {};
		desc.Count = 1;
		m_Desc.SampleDesc = desc;
		m_Desc.SampleMask = sampleMask;
	}
	void PipelineStateObject::SetRenderTargets(Uint32 numRTV, const DXGI_FORMAT* formats, DXGI_FORMAT dsvFormat)
	{
		for (Uint32 i = 0; i < numRTV; i++) {
			m_Desc.RTVFormats[i] = formats[i];
		}
		for (Uint32 i = numRTV; i < m_Desc.NumRenderTargets; i++) {
			m_Desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
		m_Desc.NumRenderTargets = numRTV;
		m_Desc.DSVFormat = dsvFormat;
		m_Desc.SampleDesc = { 1, 0 };

	}

	void PipelineStateObject::SetRenderTarget(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat)
	{
		m_Desc.NumRenderTargets = 1;
		m_Desc.RTVFormats[0] = rtvFormat;
		m_Desc.DSVFormat = dsvFormat;
	}

	void PipelineStateObject::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC depthStencilDesc)
	{
		m_Desc.DepthStencilState = depthStencilDesc;
	}

}