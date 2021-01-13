#pragma once


#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>

#include <d3dx12.h>

KRS_BEGIN_NAMESPACE(Kairos)

class CommandContext;
class RootSignature;

class DescriptorHandle {
public:
	DescriptorHandle() : m_NumHandles(0) {
		m_CPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_GPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE srcHandle, Uint32 numHandles = 1) 
		: m_CPUHandle(srcHandle)
		, m_NumHandles(numHandles)
	{
		m_GPUHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, Uint32 numHandles = 1)
		: m_CPUHandle(cpuHandle)
		, m_GPUHandle(gpuHandle)
		, m_NumHandles(numHandles)
	{}


	//void operator += (int )

	void operator+=(int offsetByDescriptorSize) {
		if (m_GPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_GPUHandle.ptr += offsetByDescriptorSize;
		if (m_CPUHandle.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			m_CPUHandle.ptr += offsetByDescriptorSize;
	}


	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_CPUHandle; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_GPUHandle; }

	Microsoft::WRL::ComPtr<ID3D12Resource> GetGPUSource() { return m_GPUResource; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;

	Uint32 m_NumHandles = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_GPUResource;
};


// each dynamic descriptor heap seems to correspond to one root signature at a time
class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(class RenderDevice* pDevice, CommandContext& context, D3D12_DESCRIPTOR_HEAP_TYPE type);
	virtual ~DynamicDescriptorHeap() = default;

	using CommitFunc = std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)>;

	void Reset();

	// copies singular CPU descriptor to GPU and returns the handle to the GPU descritpro
	D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

	void ParseGraphicsRootSignature(const RootSignature& rootSig);
	void ParseComputeRootSignature(const RootSignature& rootSig);

	// stages CPu visibl descriptors for copy to GPU visible heap (needs CommitStaged)
	// RootParameter returned from rootParamIdx must be a DESCRIPTOR_TABLE
	// offset of the descriptor table being copied to 
	void StageGraphicsDescriptorHandles(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors,
		const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle);
	void StageComputeDescriptorHandles(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors,
		const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle);

	void CommitGraphicsDescriptors() {
		CommitStagedDescriptorTables(m_GraphicsHandleCache, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
	}

	void CommitComputeDescriptors() {
		CommitStagedDescriptorTables(m_ComputeHandleCache, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
	}

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();

	class RenderDevice* m_Device;
	CommandContext& m_OwningContext;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrDescriptorHeap;
	Uint32 m_DescriptorSize; // size of each descriptor;

	// general container for all allocated discriptor heaps
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_HeapPool;
	// descriptor heaps that become reavailable
	std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_AvailableHeaps;

	
	DescriptorHandle m_CurrHandle;
	Uint32 m_NumFreeHandles;
private:


	static const Uint32 MaxDescriptorTables = 32;
	static const Uint32 MaxNumDescriptorsPerHeap = 256;

private:

	struct DescriptorTableCache
	{
		DescriptorTableCache() : NumDescriptors(0), RootDescriptor(nullptr)
		{}

		void Reset() {
			NumDescriptors = 0;
			RootDescriptor = nullptr;
		}

		Uint32 NumDescriptors;
		D3D12_CPU_DESCRIPTOR_HANDLE* RootDescriptor;
	};

	struct DescriptorHandleCache {
		DescriptorHandleCache() { ClearCache(); }
		void ClearCache() {
			m_DescriptorTableBitMask = 0;
			m_StaleDescriptorBitMask = 0;

			for (int i = 0; i < MaxDescriptorTables; ++i) {
				m_DescriptorTableCache[i].Reset();
			}
		}

		void UnbindAllValid();
		Uint32 ComputeStaleDescriptors() const;
		void ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE type, const class RootSignature& rootSig);
		void StageDescriptorHandles(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors, Uint32 descriptorSize, 
			const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle);

		DescriptorHandle m_CurrHandle;

		void CommitStagedDescriptorTables(D3D12_DESCRIPTOR_HEAP_TYPE type, Uint32 descriptorSize,
			class RenderDevice* device, class CommandContext& context, DescriptorHandle currHandleInHeap, CommitFunc setFunc);

		DescriptorTableCache m_DescriptorTableCache[MaxDescriptorTables];
		Uint32 m_DescriptorTableBitMask = 0; // which root idx in the root signature are descriptor tables
		Uint32 m_StaleDescriptorBitMask = 0; // which root idx have a staged descriptor table

		D3D12_CPU_DESCRIPTOR_HANDLE m_DescriptorHandleCache[MaxNumDescriptorsPerHeap];
	};

	DescriptorHandleCache m_GraphicsHandleCache;
	DescriptorHandleCache m_ComputeHandleCache;

private:
	bool HasSpace(Uint32 numDescriptors);
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetHeap();
	void UnbindAllValid();
	// copies stage descriptors from descriptor table from CPU -> GPU
	void CommitStagedDescriptorTables(struct DynamicDescriptorHeap::DescriptorHandleCache& handleCache, CommitFunc setFunc);
};

KRS_END_NAMESPACE