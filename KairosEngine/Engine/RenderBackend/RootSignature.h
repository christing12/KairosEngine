#pragma once



#include "Core/EngineCore.h"
#include "Core/BaseTypes.h"
#include "GraphicsTypes.h"


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

enum class ShaderRegister : Uint8
{
	ShaderResource = 1 << 1,
	ConstantBuffer = 1 << 2,
	UnorderedAcces = 1 << 3,
	Sampler = 1 << 4
};

class RootParameter {
public:
	struct RootSigLoc {
		Uint32 BaseRegister = 0;
		Uint32 RegisterSpace = 0;
		ShaderRegister RegisterType;
	};

	struct LocationHasher { size_t operator()(const RootSigLoc& key) const; };
	struct LocationEquality { size_t operator()(const RootSigLoc& left, const RootSigLoc& right) const; };

	RootParameter(D3D12_ROOT_PARAMETER_TYPE type, D3D12_SHADER_VISIBILITY = D3D12_SHADER_VISIBILITY_ALL);
	virtual ~RootParameter() {}

	void AddSignatureLocation(const RootSigLoc& location);

	inline const D3D12_ROOT_PARAMETER& D3DParamter() const { return m_Parameter;  }
	inline const std::vector<RootSigLoc>& ParamLocations() const { return m_SignatureLocations; }
protected:
	std::vector<RootSigLoc> m_SignatureLocations;
	D3D12_ROOT_PARAMETER m_Parameter;
};


class RootConstants : public RootParameter {
public:
	RootConstants(Uint32 baseRegister, Uint32 num32bitVals, Uint32 shaderRegister, D3D12_SHADER_VISIBILITY = D3D12_SHADER_VISIBILITY_ALL);
	~RootConstants() = default;
};


class RootDescriptor : public RootParameter {
public:
	RootDescriptor(Uint32 baseRegister, Uint32 registerSpace, D3D12_ROOT_PARAMETER_TYPE type, ShaderRegister shaderRegisterType,
		D3D12_SHADER_VISIBILITY = D3D12_SHADER_VISIBILITY_ALL);
	~RootDescriptor() = default;
};


class RootDescriptorTableRange
{
public:
	inline static Uint32 UnboundedRangeSize = UINT_MAX;
	KRS_CLASS_DEFAULT(RootDescriptorTableRange);
	RootDescriptorTableRange(ShaderRegister type, Uint32 baseRegister, Uint32 registerSpace, Uint32 numDescriptors = UnboundedRangeSize);

	inline const D3D12_DESCRIPTOR_RANGE& D3DRange() const { return m_Range; }

	inline const ShaderRegister& RegisterType() const { return m_Type; }
private:
	D3D12_DESCRIPTOR_RANGE m_Range;
	ShaderRegister m_Type;
};


class RootDescriptorTable : public RootParameter {
public:
	RootDescriptorTable();
	RootDescriptorTable(const RootDescriptorTable& that);
	RootDescriptorTable(RootDescriptorTable&& that);
	~RootDescriptorTable() = default;

	RootDescriptorTable& operator=(const RootDescriptorTable& that);
	RootDescriptorTable& operator=(RootDescriptorTable&& that);

	void AddDescriptorRange(const RootDescriptorTableRange& range);
private:
	std::vector<D3D12_DESCRIPTOR_RANGE> m_Ranges;

};


class RootSignature {
	friend class DynamicDescriptorHeap;
public:
	struct ParameterIndex
	{
		Uint32 index = 0;
		bool isIndirect = false; // descriptor table
	};

	KRS_CLASS_DEFAULT(RootSignature);

	RootSignature(RenderDevice* pDevice);
	ID3D12RootSignature* D3DRootSignature() const { return m_dRootSignature.Get(); }
	void Finalize(D3D12_ROOT_SIGNATURE_FLAGS flags);


	void AddDescriptorTable(const RootDescriptorTable& table);
	void AddDescriptor(const RootDescriptor& descriptor);
	void AddConstants(const RootConstants& constants);
	void AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& desc);




private:
	std::vector<RootDescriptorTable> m_DescriptorTables;
	std::vector<RootDescriptor> m_Descriptors;
	std::vector<RootConstants> m_Constants;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_StaticSamplers;
	std::vector<D3D12_ROOT_PARAMETER> m_D3DParameters;
	
	D3D12_ROOT_SIGNATURE_DESC m_Desc = {};
	std::unordered_map<
		RootParameter::RootSigLoc,
		ParameterIndex,
		RootParameter::LocationHasher,
		RootParameter::LocationEquality
	> m_ParamterIndices;

	void ValidateParamLocations(const RootParameter& parameter, bool indirect);


	RenderDevice* m_Device;


	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_dRootSignature;
	Uint32 m_DescriptorTableBitMask;
	Uint32 m_SamplerBitMask;

	// keeping track of the number of descriptors in each descriptor table root parameter
	Uint32 m_DescriptorTableSize[16] = { 0 };
};

KRS_END_NAMESPACE