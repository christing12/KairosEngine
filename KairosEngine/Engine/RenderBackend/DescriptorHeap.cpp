#include "krspch.h"
#include "DescriptorHeap.h"
#include "RenderDevice.h"

namespace Kairos {


	DescriptorAllocator::DescriptorAllocator(RenderDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE _type, Uint32 _numDescriptorsPerHeap)
		: m_Device(pDevice)
		, m_HeapType(_type)
		, m_DescriptorsPerHeap(_numDescriptorsPerHeap)
	{

	}

	DescriptorAllocator::~DescriptorAllocator()
	{
		for (Ref<DescriptorAllocatorPage> page : m_HeapPool) {
			page.reset();
		}

		m_AvailableHeaps.clear();
		m_HeapPool.clear();
	}

	DescriptorAllocation DescriptorAllocator::Allocate(Uint32 numDescriptors)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);

		DescriptorAllocation allocation;

		using FreeHeapIter = std::set<size_t>::iterator;
		FreeHeapIter iter = m_AvailableHeaps.begin();
		while (iter != m_AvailableHeaps.end()) {
			Ref<DescriptorAllocatorPage> page = m_HeapPool[*iter];

			allocation = page->Allocate(numDescriptors);
			// if descriptor page has no more free handles to get rid of from free set
			if (page->NumFreeHandles() == 0) {
				iter = m_AvailableHeaps.erase(iter);
			}
			else
				++iter;

			// if successful allocation stop searching
			if (!allocation.IsNull()) break;
		}

		// create a new page if there were no heaps available
		if (allocation.IsNull()) {
			m_DescriptorsPerHeap = std::max(m_DescriptorsPerHeap, numDescriptors);
			Ref<DescriptorAllocatorPage> page = CreateAllocatorPage();
			allocation = page->Allocate(numDescriptors);
		}
		return allocation;

	}
	void DescriptorAllocator::ReleaseStaleDescriptors()
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		for (size_t i = 0; i < m_HeapPool.size(); ++i) {
			Ref<DescriptorAllocatorPage> page = m_HeapPool[i];
			page->ReleaseStaleDescriptors();
			if (page->NumFreeHandles() > 0) {
				m_AvailableHeaps.insert(i);
			}
		}
	}

	Ref<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
	{
		Ref<DescriptorAllocatorPage> newPage = CreateRef<DescriptorAllocatorPage>(m_Device, m_HeapType, m_DescriptorsPerHeap);

		m_HeapPool.emplace_back(newPage);
		m_AvailableHeaps.insert(m_HeapPool.size() - 1);
		return newPage;
	}


	DescriptorAllocatorPage::DescriptorAllocatorPage(RenderDevice* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, Uint32 numDescriptors)

		: m_HeapType(type)
		, m_TotalDescriptors(numDescriptors)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		ZeroMemory(&heapDesc, sizeof(heapDesc));
		heapDesc.Type = m_HeapType;
		heapDesc.NumDescriptors = m_TotalDescriptors;


		KRS_CORE_ASSERT(pDevice, "Render Device Is Null");

		auto hr = pDevice->GetD3DDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_dHeap));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issuew ith creating descriptor heap page");
		m_dHeap->SetName(L"Da Heap in the Hood");

		m_NumFreeHandles = m_TotalDescriptors;
		m_CPUStartHandle = m_dHeap->GetCPUDescriptorHandleForHeapStart();
		m_GPUStartHandle = m_dHeap->GetGPUDescriptorHandleForHeapStart();
		m_HandleSize = pDevice->GetD3DDevice()->GetDescriptorHandleIncrementSize(type);

		AddNewBlock(0, m_NumFreeHandles);
	}

	bool DescriptorAllocatorPage::HasSpace(Uint32 numDescriptors) const
	{
		return m_FreeListSize.lower_bound(numDescriptors) != m_FreeListSize.end();
	}

	DescriptorAllocation DescriptorAllocatorPage::Allocate(Uint32 numDescriptors)
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		if (numDescriptors > m_NumFreeHandles) return DescriptorAllocation();

		auto smallestBlockIt = m_FreeListSize.lower_bound(numDescriptors);
		if (smallestBlockIt == m_FreeListSize.end()) return DescriptorAllocation();

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
		return DescriptorAllocation(
			CD3DX12_CPU_DESCRIPTOR_HANDLE(m_CPUStartHandle, offset, m_HandleSize),
			CD3DX12_GPU_DESCRIPTOR_HANDLE(m_GPUStartHandle, offset, m_HandleSize),
			numDescriptors, m_HandleSize, shared_from_this());
	}

	void DescriptorAllocatorPage::Free(DescriptorAllocation&& descriptor, Uint64 frameValue)
	{
		Uint32 offset = ComputeOffset(descriptor.GetDescriptorHandle());
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		m_StaleDescriptors.emplace(offset, descriptor.GetNumHandles(), frameValue);
	}

	void DescriptorAllocatorPage::ReleaseStaleDescriptors()
	{
		std::lock_guard<std::mutex> lock(m_AllocationMutex);
		while (!m_StaleDescriptors.empty()) {
			auto& staleDescriptor = m_StaleDescriptors.front();

			Uint32 offset = staleDescriptor.Offset;
			Uint32 numDescriptors = staleDescriptor.Size;

			FreeBlock(offset, numDescriptors);
			m_StaleDescriptors.pop();
		}
	}

	void DescriptorAllocatorPage::AddNewBlock(Uint32 offset, Uint32 numDescriptors)
	{
		auto offsetIt = m_FreeListOffset.emplace(offset, numDescriptors);
		auto sizeIt = m_FreeListSize.emplace(numDescriptors, offsetIt.first);
		offsetIt.first->second.FreeListBySizeIt = sizeIt;
	}


	Uint32 DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle) {
		return static_cast<Uint32>(CPUHandle.ptr - m_CPUStartHandle.ptr) / m_HandleSize;
	}


	void DescriptorAllocatorPage::FreeBlock(Uint32 offset, Uint32 numDescriptors)
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

	DescriptorAllocation::DescriptorAllocation()
		: m_CPUHandle{ 0 }
		, m_NumHandles(0)
		, m_DescriptorSize(0)
		, m_Page(nullptr)
	{
	}

	DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle,
		Uint32 numDescriptors, Uint32 handleSize, Ref<DescriptorAllocatorPage> pDescriptorPage)
		: m_CPUHandle(CPUHandle)
		, m_GPUHandle(GPUHandle)
		, m_NumHandles(numDescriptors)
		, m_DescriptorSize(handleSize)
		, m_Page(pDescriptorPage)
	{
	}

	DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation)
		: m_CPUHandle(allocation.m_CPUHandle)
		, m_NumHandles(allocation.m_NumHandles)
		, m_DescriptorSize(allocation.m_DescriptorSize)
		, m_Page(std::move(allocation.m_Page))
	{
		allocation.m_CPUHandle.ptr = 0;
		allocation.m_NumHandles = 0;
		allocation.m_DescriptorSize = 0;
	}

	DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other)
	{
		// Free this descriptor if it points to anything.
		Free();

		m_CPUHandle = other.m_CPUHandle;
		m_NumHandles = other.m_NumHandles;
		m_DescriptorSize = other.m_DescriptorSize;
		m_Page = std::move(other.m_Page);

		other.m_CPUHandle.ptr = 0;
		other.m_NumHandles = 0;
		other.m_DescriptorSize = 0;

		return *this;
	}

	DescriptorAllocation::~DescriptorAllocation()
	{
		Free();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle(Uint32 offset) const
	{
		KRS_CORE_ASSERT(offset <= m_NumHandles, "INvalid offset Num for descriptor handle");
		return { m_CPUHandle.ptr + (m_DescriptorSize * offset) };
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetGPUHandle(Uint32 offset) const
	{
		return { m_GPUHandle.ptr + (m_DescriptorSize * offset) };
	}

	void DescriptorAllocation::Free()
	{
		if (!IsNull() && m_Page) {
			m_Page->Free(std::move(*this), 3);
			m_CPUHandle.ptr = 0;
			m_NumHandles = 0;
			m_DescriptorSize = 0;
			m_Page.reset();
		}
	}




}