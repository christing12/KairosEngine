#include "krspch.h"
#include "RootSignature.h"
#include "RenderDevice.h"


namespace Kairos {

	RootParameter::RootParameter(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY visibility)
	{
		m_Parameter.ParameterType = type;
		m_Parameter.ShaderVisibility = visibility;
	}

	void RootParameter::AddSignatureLocation(const RootSigLoc& location)
	{
		m_SignatureLocations.push_back(location);
	}




	size_t RootParameter::LocationHasher::operator()(const RootSigLoc& key) const
	{
		size_t hashValue = 0;
		hashValue |= key.BaseRegister;
		hashValue <<= std::numeric_limits<decltype(key.BaseRegister)>::digits;
		hashValue |= key.RegisterSpace;
		hashValue <<= std::numeric_limits<decltype(key.RegisterSpace)>::digits;
		hashValue |= std::underlying_type_t<ShaderRegister>(key.RegisterType);
		return hashValue;
	}

	size_t RootParameter::LocationEquality::operator()(const RootSigLoc& left, const RootSigLoc& right) const
	{
		return left.BaseRegister == right.BaseRegister &&
			left.RegisterSpace == right.RegisterSpace;
	}



	RootConstants::RootConstants(Uint32 baseRegister, Uint32 num32bitVals, Uint32 registerSpace, D3D12_SHADER_VISIBILITY vis)
		: RootParameter(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, vis)
	{
		AddSignatureLocation({ baseRegister, registerSpace, ShaderRegister::ConstantBuffer });
		m_Parameter.Constants = { baseRegister, registerSpace , num32bitVals };
	}


	D3D12_DESCRIPTOR_RANGE_TYPE GetD3DDescriptorType(ShaderRegister type)
	{
		switch (type)
		{
		case ShaderRegister::ConstantBuffer: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case ShaderRegister::ShaderResource: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case ShaderRegister::UnorderedAcces: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		case ShaderRegister::Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		default:
			KRS_CORE_ERROR("WTF");
			break;
		}

		return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	}

	RootDescriptor::RootDescriptor(Uint32 baseRegister, Uint32 registerSpace, D3D12_ROOT_PARAMETER_TYPE type, ShaderRegister shaderRegisterType,
		D3D12_SHADER_VISIBILITY vis)
		: RootParameter(type, vis)
	{
		AddSignatureLocation({ baseRegister, registerSpace, shaderRegisterType });
		m_Parameter.Descriptor = { baseRegister, registerSpace };
	}


	RootDescriptorTableRange::RootDescriptorTableRange(ShaderRegister type, Uint32 baseRegister, Uint32 registerSpace, Uint32 numDescriptors)
	{
		m_Range.BaseShaderRegister = baseRegister;
		m_Range.NumDescriptors = numDescriptors;
		m_Range.RegisterSpace = registerSpace;
		m_Range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		m_Range.RangeType = GetD3DDescriptorType(type);
		m_Type = type;
	}



	RootDescriptorTable::RootDescriptorTable()
		: RootParameter(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {}

	RootDescriptorTable::RootDescriptorTable(const RootDescriptorTable& that) : RootParameter(that)
	{
		*this = that;
	}

	RootDescriptorTable::RootDescriptorTable(RootDescriptorTable&& that) : RootParameter(std::move(that))
	{
		*this = std::move(that);
	}

	RootDescriptorTable& RootDescriptorTable::operator=(const RootDescriptorTable& that)
	{
		if (this == &that) return *this;

		m_Ranges = that.m_Ranges;
		m_Parameter.DescriptorTable.NumDescriptorRanges = (UINT)m_Ranges.size();
		m_Parameter.DescriptorTable.pDescriptorRanges = m_Ranges.data();

		return *this;
	}

	RootDescriptorTable& RootDescriptorTable::operator=(RootDescriptorTable&& that)
	{
		if (this == &that) return *this;

		m_Ranges = std::move(that.m_Ranges);
		m_Parameter.DescriptorTable.NumDescriptorRanges = (UINT)m_Ranges.size();
		m_Parameter.DescriptorTable.pDescriptorRanges = m_Ranges.data();

		return *this;
	}

	void RootDescriptorTable::AddDescriptorRange(const RootDescriptorTableRange& range)
	{
	//	KRS_CORE_ASSERT(m_Ranges.empty())
		AddSignatureLocation({ (Uint16)range.D3DRange().BaseShaderRegister, (Uint16)range.D3DRange().RegisterSpace, range.RegisterType() });

		m_Ranges.push_back(range.D3DRange());
		m_Parameter.DescriptorTable.pDescriptorRanges = &m_Ranges[0];
		m_Parameter.DescriptorTable.NumDescriptorRanges = (UINT)m_Ranges.size();

	}









	RootSignature::RootSignature(RenderDevice* device)
		: m_Device(device)
	{}

	void RootSignature::AddDescriptorTable(const RootDescriptorTable& table)
	{
		ValidateParamLocations(table, true);
		m_DescriptorTables.push_back(table);
		m_D3DParameters.push_back(table.D3DParamter());
	}

	void RootSignature::AddDescriptor(const RootDescriptor& descriptor)
	{
		ValidateParamLocations(descriptor, false);
		m_Descriptors.push_back(descriptor);
		m_D3DParameters.push_back(descriptor.D3DParamter());
	}

	void RootSignature::AddConstants(const RootConstants& constants)
	{
		ValidateParamLocations(constants, false);
		m_Constants.push_back(constants);
		m_D3DParameters.push_back(constants.D3DParamter());
	}

	void RootSignature::AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& desc)
	{
		m_StaticSamplers.push_back(desc);
	}

	std::optional<RootSignature::ParameterIndex> RootSignature::GetParameterIndex(const RootParameter::RootSigLoc location) const
	{
		auto it = m_ParamterIndices.find(location);
		return it != m_ParamterIndices.end() ? std::optional(it->second) : std::nullopt;
	}

	void RootSignature::ValidateParamLocations(const RootParameter& parameter, bool indirect)
	{
		for (const auto& location : parameter.ParamLocations())
		{
			bool registerSpaceOccupied = m_ParamterIndices.find(location) != m_ParamterIndices.end();
			KRS_CORE_ASSERT(!registerSpaceOccupied, "ISSUES WITH IJWEOIJFP");
			m_ParamterIndices[location] = { (Uint32)m_D3DParameters.size(), indirect };
		}
	}


	void RootSignature::Finalize(D3D12_ROOT_SIGNATURE_FLAGS flags)
	{

		Uint32 tableParamIdx = 0;
		// need to keep track of which root parameter idexes are
		for (Uint32 param = 0; param < m_D3DParameters.size(); param++) {
			D3D12_ROOT_PARAMETER& rootParam = m_D3DParameters[param];

			if (rootParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) continue;


			{
				auto& table = m_DescriptorTables[tableParamIdx];
				rootParam.DescriptorTable.NumDescriptorRanges = table.D3DParamter().DescriptorTable.NumDescriptorRanges;
				rootParam.DescriptorTable.pDescriptorRanges = table.D3DParamter().DescriptorTable.pDescriptorRanges;
				tableParamIdx++;
			}


			//// sets bit masks and counters for dynamic descriptor heaps (moving CPU descriptors->Shader Visible)
			//{

			//	if (rootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			//		m_SamplerBitMask |= (1 << param);
			//	else
			//		m_DescriptorTableBitMask |= (1 << param);

			//	for (Uint32 i = 0; i < rootParam.DescriptorTable.NumDescriptorRanges; i++) {
			//		m_DescriptorTableSize[param] += rootParam.DescriptorTable.pDescriptorRanges[i].NumDescriptors;
			//	}
			//}

		}

		m_Desc.NumParameters = (UINT)m_D3DParameters.size();
		m_Desc.pParameters = &m_D3DParameters[0];
		m_Desc.NumStaticSamplers = (UINT)m_StaticSamplers.size();
		m_Desc.pStaticSamplers = m_StaticSamplers.data();
		m_Desc.Flags = flags;

		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;
		auto hr = D3D12SerializeRootSignature(
			&m_Desc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			rootSignatureBlob.GetAddressOf(),
			errorBlob.GetAddressOf()
		);
		if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());

		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with serializing root signature description");

		hr = m_Device->D3DDevice()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&m_dRootSignature));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with finalizing root signature");

		//m_dRootSignature->SetName(debugName.c_str());
	}
}
