#include "krspch.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "RenderDevice.h"

#include "Shader.h"

namespace Kairos {

	PipelineState::PipelineState(RenderDevice* pDevice)
		: m_Device(pDevice)
	{

	}

	void PipelineState::SetDebugName(const std::string& name)
	{
		m_DebugName = name;
	}


	GraphicsPSO::GraphicsPSO(RenderDevice* pDevice)
		: PipelineState(pDevice)
	{
		ZeroMemory(&m_Desc, sizeof(m_Desc));
		m_Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		m_Desc.RasterizerState = CD3DX12_RASTERIZER_DESC{ D3D12_DEFAULT };
		m_Desc.RasterizerState.FrontCounterClockwise = true;
		m_Desc.BlendState = CD3DX12_BLEND_DESC{ D3D12_DEFAULT };
		DXGI_SAMPLE_DESC desc = {};
		desc.Count = 1;
		m_Desc.SampleDesc = desc;
		m_Desc.NodeMask = 1;
		m_Desc.SampleMask = 0xffffffff;
		m_Desc.InputLayout.NumElements = 0;
		m_Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC{ D3D12_DEFAULT };
	}


	void GraphicsPSO::Finalize()
	{
		if (isCompiled) {
			KRS_CORE_INFO(m_DebugName + " is already compiled");
			return;
		}

		auto hr = m_Device->D3DDevice()->CreateGraphicsPipelineState(&m_Desc, IID_PPV_ARGS(&m_dPipelineState));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with finalizing pipeline state");

		m_dPipelineState->SetName(Filesystem::StringToWString(m_DebugName).c_str());
		isCompiled = true;
	}


	void GraphicsPSO::SetVertexShader(Shader* vertexShader)
	{
		m_Desc.VS.BytecodeLength = vertexShader->Code->GetBufferSize();
		m_Desc.VS.pShaderBytecode = vertexShader->Code->GetBufferPointer();
	}
	void GraphicsPSO::SetPixelShader(Shader* pixelShader)
	{
		m_Desc.PS.BytecodeLength = pixelShader->Code->GetBufferSize();
		m_Desc.PS.pShaderBytecode = pixelShader->Code->GetBufferPointer();
	}
	void GraphicsPSO::SetHullShader(Shader* hullShader)
	{
		KRS_CORE_ASSERT(hullShader != nullptr, "null shader passed for hull shader");
		m_Desc.HS.BytecodeLength = hullShader->Code->GetBufferSize();
		m_Desc.HS.pShaderBytecode = hullShader->Code->GetBufferPointer();
	}
	void GraphicsPSO::SetDomainShader(Shader* domainShader)
	{
		KRS_CORE_ASSERT(domainShader != nullptr, "null shader passed for hull shader");
		m_Desc.DS.BytecodeLength = domainShader->Code->GetBufferSize();
		m_Desc.DS.pShaderBytecode = domainShader->Code->GetBufferPointer();
	}
	void GraphicsPSO::SetRootSignature(RootSignature* sig)
	{
		m_Desc.pRootSignature = sig->D3DRootSignature();
		m_RootSig = sig;
	}
	void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc)
	{
		m_Desc.RasterizerState = rasterDesc;
	}
	void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
	{
		m_Desc.BlendState = blendDesc;
	}
	void GraphicsPSO::SetTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topology)
	{
		m_Desc.PrimitiveTopologyType = topology;
	}


	void GraphicsPSO::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layoutDesc)
	{
		m_Desc.InputLayout = layoutDesc;

	}
	void GraphicsPSO::SetSampleMask(Uint32 sampleMask)
	{
		DXGI_SAMPLE_DESC desc = {};
		desc.Count = 1;
		m_Desc.SampleDesc = desc;
		m_Desc.SampleMask = sampleMask;
	}
	void GraphicsPSO::SetRenderTargets(Uint32 numRTV, const DXGI_FORMAT* formats, DXGI_FORMAT dsvFormat)
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

	void GraphicsPSO::SetRenderTarget(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat)
	{
		m_Desc.NumRenderTargets = 1;
		m_Desc.RTVFormats[0] = rtvFormat;
		m_Desc.DSVFormat = dsvFormat;
	}

	void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC depthStencilDesc)
	{
		m_Desc.DepthStencilState = depthStencilDesc;
	}


	ComputePSO::ComputePSO(class RenderDevice* pDevice)
		: PipelineState(pDevice)
	{
		m_Desc = {};
	}


	void ComputePSO::Finalize()
	{
		if (isCompiled) {
			KRS_CORE_INFO(m_DebugName + " is already compiled");
			return;
		}
		auto hr = m_Device->D3DDevice()->CreateComputePipelineState(&m_Desc, IID_PPV_ARGS(&m_dPipelineState));
		isCompiled = true;
	}

	void ComputePSO::SetRootSignature(RootSignature* signature)
	{
		m_Desc.pRootSignature = signature->D3DRootSignature();
		m_RootSig = signature;
	}

	void ComputePSO::SetComputeShader(Shader* computeShader)
	{
		m_Desc.CS.BytecodeLength = computeShader->Code->GetBufferSize();
		m_Desc.CS.pShaderBytecode = computeShader->Code->GetBufferPointer();
	}


















}