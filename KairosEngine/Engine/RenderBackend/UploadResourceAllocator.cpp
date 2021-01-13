#include "krspch.h"
#include "RenderDevice.h"
#include "Resource.h"
#include "UploadResourceAllocator.h"

namespace Kairos {


	/* -------------------- STAGING BUFFER WITH PAGE-ALLOCATOR --------------------*/


	/* ----------------------- INDIVIDUAL RESOURCE PAGES --------------------------*/

	ResourcePage::ResourcePage(RenderDevice* pDevice, Uint32 pageSize)
		: PageSize(pageSize)
	{
		auto hr = pDevice->GetD3DDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(pageSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&Resource.NativeResource)
		);
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating staging buffer page!");
		
		Resource.ResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
		Resource.GPUAddress = Resource.NativeResource->GetGPUVirtualAddress();

		// maps resource so cpu can memcpy data to address of CPUPtr
		Resource.NativeResource->Map(0, nullptr, &CPUPtr);
	}

	ResourcePage::~ResourcePage()
	{
		// prevents address from being written into
		if (CPUPtr != nullptr)
			Resource.NativeResource->Unmap(0, nullptr);

		CPUPtr = nullptr;
		Resource = GPUResource{};
	}

	void ResourcePage::Map()
	{
		if (CPUPtr == nullptr)
			Resource.NativeResource->Map(0, nullptr, &CPUPtr);
	}

	void ResourcePage::Unmap()
	{
		if (CPUPtr != nullptr)
			Resource.NativeResource->Unmap(0, nullptr);
	}


	ResourcePage* GlobalUploadPages::GetPage(RenderDevice* pDevice, Uint32 pageSize)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		// moves all pages whose fence value have been reached back to available page queue
		while (!m_RetiredPages.empty() && pDevice->IsFenceComplete(m_RetiredPages.front().FenceValue))
		{
			ResourcePage* page = m_RetiredPages.front().Page;
			m_RetiredPages.pop();
			m_FreePages.push(page);
		}

		ResourcePage* page = nullptr;
		if (!m_FreePages.empty()) {
			page = m_FreePages.front();
			m_FreePages.pop();
		}
		else {
			m_PagePool.emplace_back(CreateScope<ResourcePage>(pDevice, pageSize));
			page = m_PagePool[m_PagePool.size() - 1].get();
		}
		return page;
	}

	void GlobalUploadPages::RetirePages(Uint64 fenceValue, const std::vector<ResourcePage*>& pages)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		for (auto iter = pages.begin(); iter != pages.end(); ++iter) {
			m_RetiredPages.push(RetiredPage{ fenceValue, *iter });
		}
	}

	void GlobalUploadPages::DeleteLargePages(RenderDevice* pDevice, Uint64 fenceValue, const std::vector<ResourcePage*>& pages)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		while (!m_RetiredLargePages.empty() && pDevice->IsFenceComplete(m_RetiredLargePages.front().FenceValue))
		{
			ResourcePage* page = m_RetiredLargePages.front().Page;
			m_RetiredLargePages.pop();
			delete page;
		}

		for (auto it = pages.begin(); it != pages.end(); ++it) {
			(*it)->Unmap();
			m_RetiredLargePages.push(RetiredPage{ fenceValue, *it });
		}
	}






	UploadResourceAllocator::UploadResourceAllocator(RenderDevice* pDevice, Uint32 sizeOfPage)
		: m_Device(pDevice)
		, m_PageSize(sizeOfPage)
		, m_CurrOffset(0)
		, m_CurrPage(nullptr)
	{

	}

	void UploadResourceAllocator::CleanupPages(Uint64 fenceValue)
	{
		if (m_CurrPage == nullptr)
			return;

		m_PagesInUse.push_back(m_CurrPage);
		m_CurrPage = nullptr;
		m_CurrOffset = 0;

		k_AllocatedPages.RetirePages(fenceValue, m_PagesInUse);
		k_AllocatedPages.DeleteLargePages(m_Device, fenceValue, m_LargePagesInUse);
		m_PagesInUse.clear();
		m_LargePagesInUse.clear();
	}

	ResourceAllocation UploadResourceAllocator::Allocate(Uint32 sizeInBytes, Uint32 alignment)
	{
		const Uint32 mask = alignment - 1;
		KRS_CORE_ASSERT((alignment & mask) == 0, "Not A Power of Two Alignment");

		Uint32 alignedSize = Math::AlignUpWithMask(sizeInBytes, alignment);
		//if (alignedSize > m_PageSize)
			

		m_CurrOffset = Math::AlignUp(m_CurrOffset, alignment);

		if (m_CurrOffset + alignment > m_PageSize)
		{
			KRS_CORE_ASSERT(m_CurrPage != nullptr, "Curr Page is null");
			m_PagesInUse.push_back(m_CurrPage);
			m_CurrPage = nullptr;
		}

		if (m_CurrPage == nullptr)
		{
			m_CurrPage = k_AllocatedPages.GetPage(m_Device);
			m_CurrOffset = 0;
		}

		ResourceAllocation alloc{
			GPUResource{m_CurrPage->Resource.GPUAddress + m_CurrOffset, m_CurrPage->Resource.NativeResource, m_CurrPage->Resource.ResourceState},
			m_CurrOffset,
			alignedSize,
			(uint8_t*)m_CurrPage->CPUPtr + m_CurrOffset
		};

		m_CurrOffset += alignedSize;

		return alloc;

	}

}
