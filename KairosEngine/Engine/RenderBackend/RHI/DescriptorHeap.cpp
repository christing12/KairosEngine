#include "krspch.h"
#include "DescriptorHeap.h"
#include "RenderDevice.h"

namespace Kairos {

	OfflineDescriptorAllocator::OfflineDescriptorAllocator(RenderDevice* pDevice, DescriptorType type, Uint32 heapSize)
		: m_Device(pDevice)
		, m_Type(type)
		, m_DescriptorsPerHeap(heapSize)
	{

	}


	DescriptorRange OfflineDescriptorAllocator::Allocate(Uint32 numDescriptors) {
		std::lock_guard<std::mutex> lock(m_Mutex);

		DescriptorRange allocation;

		using FreeHeapIter = std::set<size_t>::iterator;
		FreeHeapIter iter = m_AvailableHeaps.begin();
		while (iter != m_AvailableHeaps.end()) {
			DescriptorPage* page = m_DescriptorPagePool[*iter].get();

			allocation = page->Allocate(numDescriptors);
			// if descriptor page has no more free handles to get rid of from free set
			if (page->NumFreeHandles() == 0) {
				iter = m_AvailableHeaps.erase(iter);
			}
			else
				++iter;

			// if successful allocation stop searching
			if (!allocation.isNull()) break;
		}

		// create a new page if there were no heaps available
		if (allocation.isNull()) {
			m_DescriptorsPerHeap = std::max(m_DescriptorsPerHeap, numDescriptors);
			DescriptorPage* page = AllocateNewPage();
			allocation = page->Allocate(numDescriptors);
		}
		return allocation;

	}


	void OfflineDescriptorAllocator::ReleaseStaleDescriptors() {
		std::lock_guard<std::mutex> lock(m_Mutex);

		for (size_t i = 0; i < m_DescriptorPagePool.size(); i++) {
			DescriptorPage* page = m_DescriptorPagePool[i].get();

		}
	}
	DescriptorPage* OfflineDescriptorAllocator::AllocateNewPage()
	{
		DescriptorPage* newPage = new DescriptorPage(m_Device, m_Type, m_DescriptorsPerHeap);

		m_DescriptorPagePool.emplace_back(newPage);
		m_AvailableHeaps.insert(m_DescriptorPagePool.size() - 1);
		return newPage;
	}


	/* ------------------------ DESCRIPTOR PAGE (Represents one ID3D12DescriptorHeap) ------------------*/

	DescriptorPage::DescriptorPage(RenderDevice* pDevice, DescriptorType type, Uint32 heapSize)
		: m_Type(type)
		, m_MaxHeapSize(heapSize)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		ZeroMemory(&heapDesc, sizeof(heapDesc));
		heapDesc.Type = D3DDescriptorHeap(type);
		heapDesc.NumDescriptors = m_MaxHeapSize;

		auto hr = pDevice->D3DDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dHeap));
		m_dHeap->SetName(L"Offline Descriptor Page");

		m_NumFreeHandles = m_MaxHeapSize;
		m_StartHandle = { m_dHeap->GetCPUDescriptorHandleForHeapStart(), m_dHeap->GetGPUDescriptorHandleForHeapStart() };
		m_HandleSize = pDevice->D3DDevice()->GetDescriptorHandleIncrementSize(heapDesc.Type);

		AddNewBlock(0, m_NumFreeHandles);
	}



	bool DescriptorPage::HasSpace(Uint32 numDescriptors) const
	{
		return m_FreeListSize.lower_bound(numDescriptors) != m_FreeListSize.end();
	}

	DescriptorRange DescriptorPage::Allocate(Uint32 numDescriptors)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (numDescriptors > m_NumFreeHandles) return DescriptorRange{};

		auto smallestBlockIt = m_FreeListSize.lower_bound(numDescriptors);
		if (smallestBlockIt == m_FreeListSize.end()) return DescriptorRange();

		Uint32 blockSize = smallestBlockIt->first;

		auto offsetIt = smallestBlockIt->second;
		Uint32 offset = offsetIt->first;

		m_FreeListSize.erase(smallestBlockIt);
		m_FreeListOffset.erase(offsetIt);

		Uint32 newOffset = offset + numDescriptors;
		Uint32 newSize = blockSize - numDescriptors;
		if (newSize > 0) {
			AddNewBlock(newOffset, newSize);
		}
		m_NumFreeHandles -= numDescriptors;
		return DescriptorRange(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_StartHandle.CPUHandle(), offset, m_HandleSize),
			CD3DX12_GPU_DESCRIPTOR_HANDLE(m_StartHandle.GPUHandle(), offset, m_HandleSize),
			numDescriptors, m_HandleSize);
	}

	void DescriptorPage::Free(DescriptorRange&& descriptor, Uint64 frameValue)
	{
		Uint32 offset = ComputeOffset(descriptor.GetDescriptor(0).CPUHandle());
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_StaleDescriptors.emplace(offset, descriptor.NumDescriptors(), frameValue);
	}

	void DescriptorPage::ReleaseStaleDescriptors()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		while (!m_StaleDescriptors.empty()) {
			auto& staleDescriptor = m_StaleDescriptors.front();

			Uint32 offset = staleDescriptor.Offset;
			Uint32 numDescriptors = staleDescriptor.Size;

			FreeBlock(offset, numDescriptors);
			m_StaleDescriptors.pop();
		}
	}

	void DescriptorPage::AddNewBlock(Uint32 offset, Uint32 numDescriptors)
	{
		auto offsetIt = m_FreeListOffset.emplace(offset, numDescriptors);
		auto sizeIt = m_FreeListSize.emplace(numDescriptors, offsetIt.first);
		offsetIt.first->second.FreeListBySizeIt = sizeIt;
	}


	Uint32 DescriptorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle) {
		return static_cast<Uint32>(CPUHandle.ptr - m_StartHandle.CPUHandle().ptr) / m_HandleSize;
	}


	void DescriptorPage::FreeBlock(Uint32 offset, Uint32 numDescriptors)
	{
		// Find the first element whose offset is greater than the specified offset.
		// This is the block that should appear after the block that is being freed.
		auto nextBlockIt = m_FreeListOffset.upper_bound(offset);

		// Find the block that appears before the block being freed.
		auto prevBlockIt = nextBlockIt;
		// If it's not the first block in the list.
		if (prevBlockIt != m_FreeListOffset.begin())
		{
			// Go to the previous block in the list.
			--prevBlockIt;
		}
		else
		{
			// Otherwise, just set it to the end of the list to indicate that no
			// block comes before the one being freed.
			prevBlockIt = m_FreeListOffset.end();
		}

		// Add the number of free handles back to the heap.
		// This needs to be done before merging any blocks since merging
		// blocks modifies the numDescriptors variable.
		m_NumFreeHandles += numDescriptors;

		if (prevBlockIt != m_FreeListOffset.end() && offset == prevBlockIt->first + prevBlockIt->second.Size)
		{
			// The previous block is exactly behind the block that is to be freed.
			//
			// PrevBlock.Offset           Offset
			// |                          |
			// |<-----PrevBlock.Size----->|<------Size-------->|
			//

			// Increase the block size by the size of merging with the previous block.
			offset = prevBlockIt->first;
			numDescriptors += prevBlockIt->second.Size;

			// Remove the previous block from the free list.
			m_FreeListSize.erase(prevBlockIt->second.FreeListBySizeIt);
			m_FreeListOffset.erase(prevBlockIt);
		}

		if (nextBlockIt != m_FreeListOffset.end() && offset + numDescriptors == nextBlockIt->first)
		{
			// The next block is exactly in front of the block that is to be freed.
			//
			// Offset               NextBlock.Offset
			// |                    |
			// |<------Size-------->|<-----NextBlock.Size----->|

			// Increase the block size by the size of merging with the next block.
			numDescriptors += nextBlockIt->second.Size;

			// Remove the next block from the free list.
			m_FreeListSize.erase(nextBlockIt->second.FreeListBySizeIt);
			m_FreeListOffset.erase(nextBlockIt);
		}

		// Add the freed block to the free list.
		AddNewBlock(offset, numDescriptors);
	}











	OfflineRTVDescriptorAllocator::OfflineRTVDescriptorAllocator(RenderDevice* pDevice)
		: OfflineDescriptorAllocator(pDevice, DescriptorType::RTV)
	{}

	OfflineDSVDescriptorAllocator::OfflineDSVDescriptorAllocator(RenderDevice* pDevice)
		: OfflineDescriptorAllocator(pDevice, DescriptorType::DSV)
	{}

	OfflineCBSRVUAVDescriptorAllocator::OfflineCBSRVUAVDescriptorAllocator(RenderDevice* pDevice)
		: OfflineDescriptorAllocator(pDevice, DescriptorType::CBV)
	{}

	OfflineSamplerDescriptorAllocator::OfflineSamplerDescriptorAllocator(RenderDevice* pDevice)
		: OfflineDescriptorAllocator(pDevice, DescriptorType::Sampler)
	{}



	OnlineDescriptorHeap::OnlineDescriptorHeap(RenderDevice* pDevice, DescriptorType type, const std::vector<Uint32>& rangeSizes)
		: m_Device(pDevice)
		, m_Type(type)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		ZeroMemory(&heapDesc, sizeof(heapDesc));
		heapDesc.Type = D3DDescriptorHeap(type);
		heapDesc.NumDescriptors = std::accumulate(rangeSizes.begin(), rangeSizes.end(), 0);
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		auto hr = pDevice->D3DDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dHeap));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Failed to create online descriptor heap");
		m_HandleSize = m_Device->D3DDevice()->GetDescriptorHandleIncrementSize(heapDesc.Type);


		startCPUHandle = m_dHeap->GetCPUDescriptorHandleForHeapStart();
		startGPUHandle = m_dHeap->GetGPUDescriptorHandleForHeapStart();

		for (int rangeIdx = 0; rangeIdx < rangeSizes.size(); rangeIdx++)
		{
			Uint32 capacity = rangeSizes[rangeIdx];

			m_Ranges.emplace_back(DescriptorRange(
				{ startCPUHandle.ptr + rangeIdx * m_HandleSize * capacity },
				{ startGPUHandle.ptr + rangeIdx * m_HandleSize * capacity },
				m_HandleSize,
				capacity
			));
		}
	}


	OnlineCBVSRVUAVHeap::OnlineCBVSRVUAVHeap(RenderDevice* pDevice, const std::vector<Uint32>& rangeSizes)
		: OnlineDescriptorHeap(pDevice, DescriptorType::CBV, rangeSizes)
		, m_CBVPool(1, 1000)
		, m_SRVPool(1, 1000)
		, m_UAVPool(1, 1000)
	{}


	OnlineCBVSRVUAVHeap::Pool::Pool(Uint32 slotSize, Uint32 growthSize)
		: m_Growth(growthSize)
		, m_SlotSize(slotSize)
	{}

	void OnlineCBVSRVUAVHeap::Pool::Grow() {
		for (auto i = 0u; i < m_Growth; ++i)
		{
			m_FreeSlots.emplace_back(m_AllocateSize);
			m_AllocateSize += m_SlotSize;
		}
	}

	Uint32 OnlineCBVSRVUAVHeap::Pool::Allocate()
	{
		if (m_FreeSlots.empty())
		{
			Grow();
		}

		Uint32 slot = m_FreeSlots.front();
		m_FreeSlots.pop_front();
		return slot;
	}

	DescriptorRange OnlineCBVSRVUAVHeap::AllocateSRV()
	{
		Uint32 slot = m_SRVPool.Allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUAddress(slot, Range::SRV);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUAddress(slot, Range::SRV);

		return DescriptorRange(cpuHandle, gpuHandle, m_HandleSize, 1, slot);
	}

	DescriptorRange OnlineCBVSRVUAVHeap::AllocateCBV()
	{
		Uint32 slot = m_CBVPool.Allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUAddress(slot, Range::CBV);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUAddress(slot, Range::CBV);

		return DescriptorRange(cpuHandle, gpuHandle, m_HandleSize, 1, slot);
	}

	DescriptorRange OnlineCBVSRVUAVHeap::AllocateUAV()
	{
		Uint32 slot = m_UAVPool.Allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUAddress(slot, Range::UAV);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUAddress(slot, Range::UAV);

		return DescriptorRange(cpuHandle, gpuHandle, m_HandleSize, 1, slot);
	}


	D3D12_CPU_DESCRIPTOR_HANDLE OnlineDescriptorHeap::GetCPUAddress(Uint32 indexInRange, Uint32 rangeIdx)
	{
		return { m_Ranges[rangeIdx].CPUHandle().ptr + indexInRange * m_HandleSize };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE OnlineDescriptorHeap::GetGPUAddress(Uint32 indexInRange, Uint32 rangeIdx)
	{
		return { m_Ranges[rangeIdx].GPUHandle().ptr + indexInRange * m_HandleSize };

	}

	Descriptor OnlineDescriptorHeap::RangeStart(OnlineCBVSRVUAVHeap::Range rangeType)
	{
		auto index = std::underlying_type_t<Range>(rangeType);
		auto& range = GetRange(index);
		return range.GetDescriptor(0);
	}

	DescriptorRange& OnlineDescriptorHeap::GetRange(Uint32 index)
	{
		return m_Ranges.at(index);
	}




	OnlineSamplerHeap::OnlineSamplerHeap(RenderDevice* pDevice, Uint32 rangeSize)
		: OnlineDescriptorHeap(pDevice, DescriptorType::Sampler, { rangeSize })
		, m_SamplerPool{ 1, 1000 }
	{}

	DescriptorRange OnlineSamplerHeap::AllocateSampler()
	{
		Uint32 slot = m_SamplerPool.Allocate();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCPUAddress(slot, 0);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = GetGPUAddress(slot, 0);

		return DescriptorRange(cpuHandle, gpuHandle, m_HandleSize, 1, slot);
	}











	PoolDescriptorAllocator::Desc::Desc(const std::vector<Uint32> rangeCount, bool shaderVisible)
	{
		KRS_CORE_ASSERT(rangeCount.size() <= (Uint32)DescriptorType::Count, " Too Many Ranges listed for initialization");
		for (size_t type = 0; type < rangeCount.size(); ++type)
		{
			m_DescriptorRangeCount[type] = rangeCount[type];
		}
		isShaderVisible = shaderVisible;
	}








}

