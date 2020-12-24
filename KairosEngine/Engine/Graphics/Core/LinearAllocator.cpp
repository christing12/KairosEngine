#include "krspch.h"
#include "LinearAllocator.h"
#include "RenderDevice.h"
#include "Resource.h"

namespace Kairos {
	LinearAllocator::LinearAllocator(RenderDevice* pDevice, LINEAR_ALLOCATOR_TYPE type)
		: m_Device(pDevice)
		, m_InternalType(type)
	{
		m_PageSize = type == CPU_WRITABLE ? _128MB : _64KB;
	}
	void LinearAllocator::Reset()
	{
		m_CurrPage = nullptr;
		m_FreePages = m_PagePool;
		for (auto page : m_FreePages) {
			page->Reset();
		}
	}

	LinearAllocator::Allocation LinearAllocator::Allocate(size_t sizeInBytes, size_t alignment)
	{
		//KRS_CORE_ASSERT(sizeInBytes <= m_PageSize, "Size in Bytes exceeds page size (2MB?)");
		if (!m_CurrPage || !m_CurrPage->HasSpace(sizeInBytes, alignment)) {
			m_CurrPage = RequestPage();
		}
		return m_CurrPage->Allocate(sizeInBytes, alignment);
	}

	Ref<LinearAllocator::Page> LinearAllocator::RequestPage()
	{
		Ref<Page> page;
		if (!m_FreePages.empty()) {
			page = m_FreePages.front();
			m_FreePages.pop_front();
		}
		else {
			page = CreateRef<Page>(m_PageSize, m_Device);
		}
		return page;
	}



	LinearAllocator::Page::Page(size_t _pageSize, RenderDevice* pDevice)
		: m_Size(_pageSize)
		, m_CurrOffset(0)
		, m_CPUPtr(nullptr)
		, m_GPUPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
	{
		ID3D12Device2* device = pDevice->GetD3DDevice();
		D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D3D12_RESOURCE_DESC desc = Resource::CreateBufferDesc(_pageSize);

		auto hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_dResource)
		);

		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating a new ID3D12Heap");
		m_GPUPtr = m_dResource->GetGPUVirtualAddress();
		m_dResource->Map(0, nullptr, &m_CPUPtr); // retrieves cpu address

	}
	LinearAllocator::Page::~Page()
	{
		m_dResource->Unmap(0, nullptr);
		m_CPUPtr = nullptr;
		m_GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
	}

	void LinearAllocator::Page::Reset()
	{
		m_CurrOffset = 0;
	}
	bool LinearAllocator::Page::HasSpace(size_t sizeInBytes, size_t alignment) const
	{
		size_t alignedSize = Math::AlignUp(sizeInBytes, alignment);
		size_t alignedOffset = Math::AlignUp(m_CurrOffset, alignment);
		return alignedOffset + alignedSize <= m_Size;
	}
	LinearAllocator::Allocation LinearAllocator::Page::Allocate(size_t sizeInbytes, size_t alignment)
	{
		KRS_CORE_ASSERT(HasSpace(sizeInbytes, alignment), "not enough space in page");

		size_t alignedSize = Math::AlignUp(sizeInbytes, alignment);
		m_CurrOffset = Math::AlignUp(m_CurrOffset, alignment);
		Allocation allocation;
		allocation.CPU = static_cast<Uint8*>(m_CPUPtr) + m_CurrOffset;
		allocation.GPU = m_GPUPtr + m_CurrOffset;
		allocation.UploadResource = m_dResource;

		m_CurrOffset += alignedSize;
		return allocation;
	}
}