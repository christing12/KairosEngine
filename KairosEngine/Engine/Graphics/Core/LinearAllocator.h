#pragma once

#include "GraphicsTypes.h"

namespace Kairos {
	class RenderDevice;

	class LinearAllocator {
	public:
		// used for copying CPU memory -> GPU virtual address
		struct Allocation {
			void* CPU = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS GPU = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
			Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource = nullptr;
		};

	public:
		LinearAllocator(class RenderDevice* pDevice, LINEAR_ALLOCATOR_TYPE type);
		size_t GetPageSize() const { return m_PageSize; }
		void Reset(); // release alocated pages (when done executing on command queue)

		// allocated memory < size of page
		// memcpy to void* CPU
		Allocation Allocate(size_t sizeInBytes, size_t alignment = 256);

	private:
		LINEAR_ALLOCATOR_TYPE m_InternalType;

		struct Page {
		public:
			Page() {}
			Page(size_t _pageSize, class RenderDevice* pDevice);
			~Page();

			void Reset();
			bool HasSpace(size_t sizeInBytes, size_t alignment) const;
			Allocation Allocate(size_t sizeInbytes, size_t alignment);
		private:
			Microsoft::WRL::ComPtr<ID3D12Resource> m_dResource; // GPU memory
			D3D12_GPU_VIRTUAL_ADDRESS	m_GPUPtr = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
			void* m_CPUPtr = nullptr;

			// page size in bytes
			size_t m_Size = 0;
			// offset in bytes
			size_t m_CurrOffset = 0;
		};
		class RenderDevice* m_Device;


		using PagePool = std::deque<Ref<Page>>;
		PagePool m_PagePool;
		PagePool m_FreePages;

		Ref<Page> m_CurrPage = nullptr;
		Ref<Page> RequestPage();
		size_t m_PageSize = 0;
	};
}