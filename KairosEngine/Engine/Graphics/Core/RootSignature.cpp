#include "krspch.h"
#include "RootSignature.h"
#include "RenderDevice.h"


namespace Kairos {

	RootSignature::RootSignature(RenderDevice* pDevice, Uint32 numParams, Uint32 numStaticSampler)
		: m_Device(pDevice)
	{
		Reset(numParams, numStaticSampler);
	}

	RootSignature::~RootSignature()
	{
	}

	void RootSignature::InitStaticSampler(Uint32 Register, const D3D12_SAMPLER_DESC& nonStaticSampler, D3D12_SHADER_VISIBILITY visibility)
	{
		D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = m_SamplerArray[m_NumInitSamplers++];

		StaticSamplerDesc.Filter = nonStaticSampler.Filter;
		StaticSamplerDesc.AddressU = nonStaticSampler.AddressU;
		StaticSamplerDesc.AddressV = nonStaticSampler.AddressV;
		StaticSamplerDesc.AddressW = nonStaticSampler.AddressW;
		StaticSamplerDesc.MipLODBias = nonStaticSampler.MipLODBias;
		StaticSamplerDesc.MaxAnisotropy = nonStaticSampler.MaxAnisotropy;
		StaticSamplerDesc.ComparisonFunc = nonStaticSampler.ComparisonFunc;
		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		StaticSamplerDesc.MinLOD = nonStaticSampler.MinLOD;
		StaticSamplerDesc.MaxLOD = nonStaticSampler.MaxLOD;
		StaticSamplerDesc.ShaderRegister = Register;
		StaticSamplerDesc.RegisterSpace = 0;
		StaticSamplerDesc.ShaderVisibility = visibility;

		//if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		//	StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		//	StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
		//{
		//	if (nonStaticSampler.BorderColor[3] == 1.0f)
		//	{
		//		if (nonStaticSampler.BorderColor[0] == 1.0f)
		//			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		//		else
		//			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		//	}
		//	else
		//		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		//}
	}

	void RootSignature::SetStaticSampler(Uint32 Register, const D3D12_STATIC_SAMPLER_DESC& staticSampler)
	{
		m_SamplerArray[m_NumInitSamplers++] = staticSampler;
	}

	void RootSignature::Reset(Uint32 numParams, Uint32 numSamplers)
	{
		if (numParams)
			m_ParamArray.reset(new RootParameter[numParams]);
		else
			m_ParamArray = nullptr;

		if (numSamplers)
			m_SamplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[numSamplers]);
		else
			m_SamplerArray = nullptr;

		m_NumParams = numParams;
		m_NumStaticSampler = numSamplers;
		m_NumInitSamplers = 0;
	}
	void RootSignature::Finalize(const std::wstring& debugName, D3D12_ROOT_SIGNATURE_FLAGS flags)
	{
		D3D12_ROOT_SIGNATURE_DESC rootDesc;
		rootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)m_ParamArray.get();
		rootDesc.NumStaticSamplers = m_NumStaticSampler;
		rootDesc.NumParameters = m_NumParams;
		rootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)m_SamplerArray.get();
		rootDesc.Flags = flags;

		// update bitmask information
		m_DescriptorTableBitMask = 0;
		m_SamplerBitMask = 0;

		// need to keep track of which root parameter idexes are
		for (Uint32 param = 0; param < m_NumParams; param++) {
			const D3D12_ROOT_PARAMETER& rootParam = rootDesc.pParameters[param];
			if (rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {

				if (rootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
					m_SamplerBitMask |= (1 << param);
				else
					m_DescriptorTableBitMask |= (1 << param);

				for (Uint32 i = 0; i < rootParam.DescriptorTable.NumDescriptorRanges; i++) {
					m_DescriptorTableSize[param] += rootParam.DescriptorTable.pDescriptorRanges[i].NumDescriptors;
				}
			}
		}

		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		auto hr = D3D12SerializeRootSignature(
			&rootDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			rootSignatureBlob.GetAddressOf(),
			errorBlob.GetAddressOf()
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with serializing root signature description");

		hr = m_Device->GetD3DDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_dRootSignature));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with finalizing root signature");

		m_dRootSignature->SetName(debugName.c_str());
	}
}