#pragma once


#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

#include <map>
#include <set>
#include <queue>

#include <wrl.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class DescriptorAllocation {
public:
	DescriptorAllocation();
	DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle,
		Uint32 numDescriptors, Uint32 numHandles, Ref<class DescriptorAllocatorPage> pDescriptorPage);
	DescriptorAllocation(const DescriptorAllocation& other) = default;
	DescriptorAllocation(DescriptorAllocation&& allocation);
	DescriptorAllocation& operator=(DescriptorAllocation&& other);
	~DescriptorAllocation();

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(Uint32 offset = 0) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(Uint32 offset = 0) const;
	Uint32 GetNumHandles() const { return m_NumHandles; }
	Ref<class DescriptorAllocatorPage> GetPage() { return m_Page; }
	bool IsNull() const { return m_CPUHandle.ptr == 0; }

private:
	void Free();

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
	Uint32 m_NumHandles = 0;
	Uint32 m_DescriptorSize = 0;
	Ref<struct DescriptorAllocatorPage> m_Page;
};


struct DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
	DescriptorAllocatorPage(RenderDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, Uint32 numDescriptors);
	~DescriptorAllocatorPage() { m_dHeap = nullptr;  m_CPUStartHandle.ptr = 0; }

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_HeapType; }
	ID3D12DescriptorHeap* GetHeap() { return m_dHeap.Get(); }
	Uint32 NumFreeHandles() const { return m_NumFreeHandles; }

	bool HasSpace(Uint32 numDescriptors) const;
	DescriptorAllocation Allocate(Uint32 numDescriptors);
	void Free(DescriptorAllocation&& descriptorHandle, Uint64 frameValue);
	void ReleaseStaleDescriptors();
protected:
	Uint32 ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void AddNewBlock(Uint32 offset, Uint32 numDescriptors);
	void FreeBlock(Uint32 offset, Uint32 numDescriptors);

private:
	struct FreeBlockInfo;
	using FreeListByOffset = std::map<Uint32, FreeBlockInfo>;
	using FreeListBySize = std::multimap<Uint32, FreeListByOffset::iterator>;
	struct FreeBlockInfo {
		FreeBlockInfo(Uint32 _size)
			: Size(_size)
		{}


		Uint32 Size;
		FreeListBySize::iterator FreeListBySizeIt;
	};

	struct StaleDescriptorInfo {
		StaleDescriptorInfo(Uint32 _offset, Uint32 _size, Uint64 _frame)
			: Offset(_offset)
			, Size(_size)
			, FrameNumber(_frame)
		{}
		Uint32 Offset = 0;
		Uint32 Size = 0;
		Uint64 FrameNumber = 0;
	};
	using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;


private:
	FreeListByOffset m_FreeListOffset;
	FreeListBySize m_FreeListSize;
	StaleDescriptorQueue m_StaleDescriptors;


	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	Uint32 m_TotalDescriptors = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dHeap;

	Uint32 m_NumFreeHandles = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GPUStartHandle;
	Uint32 m_HandleSize = 0;

	std::mutex m_AllocationMutex;
};

class DescriptorAllocator {
public:
	DescriptorAllocator() = default;
	DescriptorAllocator(class RenderDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE _type, Uint32 _numDescriptorsPerHeap = 256);
	~DescriptorAllocator();

	DescriptorAllocation Allocate(Uint32 numDescriptors = 1); // allocates contiguous memory from heap
	void ReleaseStaleDescriptors();
private:
	using FreeHeaps = std::set<size_t>;
	using AllocatedPages = std::vector<Ref<DescriptorAllocatorPage>>;

private:
	RenderDevice* m_Device;
	
	// helper function to create new paged descriptor heap to suballoate from
	Ref<DescriptorAllocatorPage> CreateAllocatorPage();


	D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
	AllocatedPages m_HeapPool;
	FreeHeaps m_AvailableHeaps;
	Uint32 m_DescriptorsPerHeap = 0;

	std::mutex m_AllocationMutex;
};

KRS_END_NAMESPACE