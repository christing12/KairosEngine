#pragma once

KRS_BEGIN_NAMESPACE(Kairos)

class CommandContext;
class RootSignature;


// each dynamic descriptor heap seems to correspond to one root signature at a time
class DynamicDescriptorHeap {
public:
	DynamicDescriptorHeap(class RenderDevice* pDevice, CommandContext& context, D3D12_DESCRIPTOR_HEAP_TYPE type,
		Uint32 numDescriptorsInHeap = 1024);
	virtual ~DynamicDescriptorHeap() = default;

	void Reset();

	// copies singular CPU descriptor to GPU and returns the handle to the GPU descritpro
	D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);

	void ParseRootSignature(const RootSignature& rootSig);

	Uint32 ComputeStaleDescriptors() const;

	// stages CPu visibl descriptors for copy to GPU visible heap (needs CommitStaged)
	// RootParameter returned from rootParamIdx must be a DESCRIPTOR_TABLE
	// offset of the descriptor table being copied to 
	void StageDescriptorTable(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors,
		const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle);

	void StageCBV(Uint32 rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAddress);


	// copies stage descriptors from descriptor table from CPU -> GPU
	void CommitStagedDescriptors();

private:
	void CommitStagedDescriptorTables();
	class RenderDevice* m_Device;
	CommandContext& m_OwningContext;
	D3D12_DESCRIPTOR_HEAP_TYPE m_Type;

	
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_CurrCPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_CurrGPUHandle;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrDescriptorHeap;

	Uint32 m_NumFreeHandles;

	Uint32 m_NumDescriptorsPerHeap;
	Uint32 m_DescriptorSize; // size of each descriptor;

	Scope<D3D12_CPU_DESCRIPTOR_HANDLE[]> m_DescriptorHandleCache;

	// general container for all allocated discriptor heaps
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_HeapPool;
	// descriptor heaps that become reavailable
	std::queue<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_AvailableHeaps;

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();

	static const Uint32 MaxDescriptorTables = 32;

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

	DescriptorTableCache m_DescriptorTableCache[MaxDescriptorTables]; // filled out by ParseRootSignature
	D3D12_GPU_VIRTUAL_ADDRESS m_CBVCache[MaxDescriptorTables];


	Uint32 m_DescriptorTableBitMask = 0;
	Uint32 m_StaleDescriptorBitMask = 0;
	Uint32 m_CBVBitMask = 0;
};

KRS_END_NAMESPACE