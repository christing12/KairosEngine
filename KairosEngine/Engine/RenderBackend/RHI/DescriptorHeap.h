#pragma once

#include "Descriptor.h"
#include "GraphicsTypes.h"
#include <set>
#include <queue>
#include <map>


#define DEFAULT_OFFLINE_HEAP_SIZE 256


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

class DescriptorPage {
public:
	DescriptorPage() = default;
	DescriptorPage(RenderDevice* pDevice, DescriptorType type, Uint32 heapSize);


	Uint32 NumFreeHandles() const { return m_NumFreeHandles; }

	bool HasSpace(Uint32 numDescriptors) const;
	DescriptorRange Allocate(Uint32 numDescriptors);
	void Free(DescriptorRange&& descriptorHandle, Uint64 frameValue);
	void ReleaseStaleDescriptors();


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

	Uint32 ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void AddNewBlock(Uint32 offset, Uint32 numDescriptors);
	void FreeBlock(Uint32 offset, Uint32 numDescriptors);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dHeap;
	Uint32 m_MaxHeapSize;
	DescriptorType m_Type;
	Descriptor m_StartHandle;
	Uint32 m_NumFreeHandles;
	Uint32 m_HandleSize;
	std::mutex m_Mutex;

	FreeListByOffset m_FreeListOffset;
	FreeListBySize m_FreeListSize;
	StaleDescriptorQueue m_StaleDescriptors;

public:
	inline ID3D12DescriptorHeap* D3DHeap() { return m_dHeap.Get(); }
	inline const ID3D12DescriptorHeap* D3DHeap() const { return m_dHeap.Get(); }
	inline DescriptorType HeapType() const { return m_Type; }
};







// paged free list allocator
class OfflineDescriptorAllocator {
public:
	using AllocatedPages = std::vector<Scope<DescriptorPage>>;
	OfflineDescriptorAllocator() = default;
	OfflineDescriptorAllocator(RenderDevice* pDevice, DescriptorType type, Uint32 heapSize = DEFAULT_OFFLINE_HEAP_SIZE);

	DescriptorRange Allocate(Uint32 numDescriptors = 1);
	void ReleaseStaleDescriptors();
protected:
	DescriptorPage* AllocateNewPage();
protected:
	RenderDevice* m_Device;
	DescriptorType m_Type;
	Uint32 m_DescriptorsPerHeap;

	AllocatedPages m_DescriptorPagePool;
	std::set<size_t> m_AvailableHeaps;

	std::mutex m_Mutex;
};

class OfflineRTVDescriptorAllocator : public OfflineDescriptorAllocator {
public:
	OfflineRTVDescriptorAllocator() = default;

	OfflineRTVDescriptorAllocator(RenderDevice* pDevice);
	~OfflineRTVDescriptorAllocator() = default;
};

class OfflineDSVDescriptorAllocator : public OfflineDescriptorAllocator {
public:
	OfflineDSVDescriptorAllocator() = default;

	OfflineDSVDescriptorAllocator(RenderDevice* pDevice);
	~OfflineDSVDescriptorAllocator() = default;
};

class OfflineCBSRVUAVDescriptorAllocator : public OfflineDescriptorAllocator {
public:
	OfflineCBSRVUAVDescriptorAllocator() = default;
	OfflineCBSRVUAVDescriptorAllocator(RenderDevice* pDevice);
	~OfflineCBSRVUAVDescriptorAllocator() = default;
};

class OfflineSamplerDescriptorAllocator : public OfflineDescriptorAllocator {
public:
	OfflineSamplerDescriptorAllocator() = default;
	OfflineSamplerDescriptorAllocator(RenderDevice* pDevice);
	~OfflineSamplerDescriptorAllocator() = default;
};




class OnlineDescriptorHeap {
public:
	OnlineDescriptorHeap() = default;

	OnlineDescriptorHeap(RenderDevice* m_Device, DescriptorType type, const std::vector<Uint32>& rangeSizes);
	virtual ~OnlineDescriptorHeap() {}

	class Pool {
	public:
		Pool() = default;
		Pool(Uint32 slotSize, Uint32 growthCount);
		//TODO: Add Deallocation
		Uint32 Allocate();
	private:
		std::list<Uint32> m_FreeSlots;
		void Grow();
		Uint32 m_Growth = 1000;
		Uint32 m_SlotSize = 1;
		Uint32 m_AllocateSize = 0;

	};
	enum Range {
		SRV = 0,
		CBV = 1,
		UAV = 2
	};
	Descriptor RangeStart(Range rangeType);

protected:
	RenderDevice* m_Device;
	DescriptorType m_Type;
	Uint32 m_HandleSize;

	D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle;

	static const Uint32 RANGE_CAPACITY = 1000;
	std::vector<DescriptorRange> m_Ranges;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dHeap;


	DescriptorRange& GetRange(Uint32 index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(Uint32 indexInRange, Uint32 rangeIdx);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress(Uint32 indexInRange, Uint32 rangeIdx);
public:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> D3DHeap() { return m_dHeap; }
	Descriptor GetStart() const { return { startCPUHandle, startGPUHandle }; }
};


class OnlineCBVSRVUAVHeap : public OnlineDescriptorHeap {
public:
	OnlineCBVSRVUAVHeap() {}
	OnlineCBVSRVUAVHeap(RenderDevice* pDevice, const std::vector<Uint32>& rangeSizes);
	
	
	DescriptorRange AllocateSRV();
	DescriptorRange AllocateCBV();
	DescriptorRange AllocateUAV();

private:
	Pool m_CBVPool;
	Pool m_SRVPool;
	Pool m_UAVPool;
};


class OnlineSamplerHeap : public OnlineDescriptorHeap {
public:
	OnlineSamplerHeap() {}
	
	OnlineSamplerHeap(RenderDevice* pDevice, Uint32 rangeSize);
	DescriptorRange AllocateSampler();
private:
	Pool m_SamplerPool;
};




class PoolDescriptorAllocator {
public:
	struct Desc {
		Desc() = default;
		Desc(const std::vector<Uint32> rangeCounts, bool shaderVisible = false);
		Desc& SetRangeCount(DescriptorType type, Uint32 count);
		Desc& SetShaderVisible(bool isVisible);
	private:
		bool isShaderVisible = false;
		// how many descriptors of each type can be allocated in their respective heaps
		Uint32 m_DescriptorRangeCount[(Uint32)DescriptorType::Count] = { 0 };
	};

	PoolDescriptorAllocator() = default;
	PoolDescriptorAllocator(const PoolDescriptorAllocator::Desc& desc);


private:
	RenderDevice* m_Device;
	PoolDescriptorAllocator::Desc& m_Desc;
};




KRS_END_NAMESPACE