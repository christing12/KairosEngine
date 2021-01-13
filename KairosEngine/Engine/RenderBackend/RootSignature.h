#pragma once



#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

struct RootParameter {
	friend class RootSignature;
	RootParameter() {
		m_Parameter.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	~RootParameter() {
		Clear();
	}

	void Clear() {
		if (m_Parameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			delete[] m_Parameter.DescriptorTable.pDescriptorRanges;

		m_Parameter.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	void InitAsConstants(Uint32 registerNum, Uint32 num32BitVal, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, Uint32 registerSpace = 0) {
		m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		m_Parameter.ShaderVisibility = visibility;
		m_Parameter.Constants.Num32BitValues = num32BitVal;
		m_Parameter.Constants.RegisterSpace = registerSpace;
		m_Parameter.Constants.ShaderRegister = registerNum;
	}

	void InitAsConstantBuffer(Uint32 shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, Uint32 registerSpace = 0) {
		m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_Parameter.ShaderVisibility = visibility;
		m_Parameter.Descriptor.ShaderRegister = shaderRegister;
		m_Parameter.Descriptor.RegisterSpace = registerSpace;
	}

	void InitAsDescriptorTable(Uint32 rangeCount, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) {
		m_Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_Parameter.ShaderVisibility = visibility;
		m_Parameter.DescriptorTable.NumDescriptorRanges = rangeCount;
		m_Parameter.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
	}

	void SetDescriptorRange(Uint32 descriptorTableIndex, D3D12_DESCRIPTOR_RANGE_TYPE type,
		Uint32 numDescriptors, Uint32 baseRegister, Uint32 registerSpace = 0) {

		KRS_CORE_ASSERT(descriptorTableIndex < m_Parameter.DescriptorTable.NumDescriptorRanges, "table index not valid");
		D3D12_DESCRIPTOR_RANGE* descriptorRange = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_Parameter.DescriptorTable.pDescriptorRanges + descriptorTableIndex);
		descriptorRange->RangeType = type;
		descriptorRange->NumDescriptors = numDescriptors;
		descriptorRange->BaseShaderRegister = baseRegister;
		descriptorRange->RegisterSpace = registerSpace;
		descriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	const D3D12_ROOT_PARAMETER& operator()() const { return m_Parameter; }
private:
	D3D12_ROOT_PARAMETER m_Parameter;

};



class RootSignature {
	friend class DynamicDescriptorHeap;
public:
	RootSignature(RenderDevice* pDevice, Uint32 numParams, Uint32 numStaticSampler = 0);
	ID3D12RootSignature* GetD3DRootSignature() const { return m_dRootSignature.Get(); }

	void SetRootParameters(Uint32 numParams, Uint32 offset, RootParameter params[]);

	~RootSignature();

	void InitStaticSampler(Uint32 Register, const D3D12_SAMPLER_DESC& nonStaticSampler,
		D3D12_SHADER_VISIBILITY visibility);

	void SetStaticSampler(Uint32 Register, const D3D12_STATIC_SAMPLER_DESC& staticSampler);

	RootParameter& operator[] (size_t entryIdx) {
		KRS_CORE_ASSERT(entryIdx < m_NumParams, "Issue with access root parameters");
		return m_ParamArray[entryIdx];
	}
	const RootParameter& operator[](size_t entryIdx) const {
		KRS_CORE_ASSERT(entryIdx < m_NumParams, "Issue with access const& root parameter");
		return m_ParamArray[entryIdx];
	}

	void Reset(Uint32 numParams, Uint32 numSamplers = 0);
	void Finalize(const std::wstring& debugName, D3D12_ROOT_SIGNATURE_FLAGS flags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
	);
private:
	RenderDevice* m_Device;
	Uint32 m_NumParams;
	Uint32 m_NumStaticSampler;
	Uint32 m_NumInitSamplers = 0;
	bool m_Compiled = false;

	Ref<RootParameter[]> m_ParamArray;
	Ref<D3D12_STATIC_SAMPLER_DESC[]> m_SamplerArray;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_dRootSignature;

	Uint32 m_DescriptorTableBitMask;
	Uint32 m_SamplerBitMask;
	Uint32 m_DescriptorTableSize[16] = { 0 }; // keeping track of the number of descriptors in each descriptor table root parameter
private:

};

KRS_END_NAMESPACE