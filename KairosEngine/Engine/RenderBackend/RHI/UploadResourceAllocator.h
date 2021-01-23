#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>
#include "GraphicsTypes.h"

#include <queue>


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;

struct DynAlloc {
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
	GPUAddress gpuAdress;
	Uint32 Offset;
	Uint32 Size;
	void* CPUPtr;
};

struct ResourcePage {
	ResourcePage() = default;
	~ResourcePage();

	void Map();
	void Unmap();

	ResourcePage(RenderDevice* pDevice, Uint32 pageSize);

	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	GPUAddress startGPUAddress;
	void* CPUPtr;
	Uint32 PageSize;
};

class GlobalUploadPages {
public:
	struct RetiredPage {
		Uint64 FenceValue;
		ResourcePage* Page;
	};

	GlobalUploadPages() = default;
	~GlobalUploadPages() = default;

	using AllocatedPages = std::vector<Kairos::Scope<ResourcePage>>;
	using FencedPages = std::queue<RetiredPage>;

	ResourcePage* GetPage(RenderDevice* pDevice, Uint32 pageSize = _2MB);

	void Destroy();

	void RetirePages(Uint64 fenceValue, const std::vector<ResourcePage*>& pages);
	void DeleteLargePages(RenderDevice* pDevice, Uint64 fenceValue, const std::vector<ResourcePage*>& pages);

private:

	AllocatedPages m_PagePool;
	// upload pages whose work have been submitted to the GPU (potentially not ready to use yet b/c GPU hasn't reached fence
	FencedPages m_RetiredPages;
	FencedPages m_RetiredLargePages;
	// upload pages whose work on the GPU has been completed and fence value have been checked
	std::queue<ResourcePage*> m_FreePages;

	// to lock so only thread can get new pages at a time
	std::mutex m_Mutex;
};

class UploadResourceAllocator {
public:
	KRS_CLASS_DEFAULT(UploadResourceAllocator);

	UploadResourceAllocator(RenderDevice* pDevice, Uint32 sizeOfPage = _4MB);

	void CleanupPages(Uint64 fenceValue);
	DynAlloc Allocate(Uint32 sizeInBytes, Uint32 alignment = 256);

	static void Destroy() { k_AllocatedPages.Destroy(); }

	Uint32 PageSize() const { return m_PageSize; }


private:
	DynAlloc AllocateLargePage(Uint32 alignedSize);
	static GlobalUploadPages k_AllocatedPages;
private:
	RenderDevice* m_Device;
	Uint32 m_PageSize = 0;
	Uint32 m_CurrOffset = 0;
	ResourcePage* m_CurrPage = nullptr;


	std::vector<ResourcePage*> m_PagesInUse;
	std::vector<ResourcePage*> m_LargePagesInUse;
};

KRS_END_NAMESPACE