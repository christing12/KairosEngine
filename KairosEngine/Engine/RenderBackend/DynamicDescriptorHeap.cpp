#include "krspch.h"
#include "DynamicDescriptorHeap.h"
#include "RenderDevice.h"
#include "RootSignature.h"
#include "CommandContext.h"

namespace Kairos {
	DynamicDescriptorHeap::DynamicDescriptorHeap(RenderDevice* pDevice, CommandContext& context,
		D3D12_DESCRIPTOR_HEAP_TYPE type)
		: m_Device(pDevice)
		, m_OwningContext(context)
		, m_Type(type)
		, m_NumFreeHandles(0)
		, m_GraphicsHandleCache()
		, m_ComputeHandleCache()
	{
		// queries for descriptor size in bytes (varies by vendor)
		m_DescriptorSize = m_Device->GetD3DDevice()->GetDescriptorHandleIncrementSize(type);
	}

	void DynamicDescriptorHeap::Reset()
	{
		std::queue<ComPtr<ID3D12DescriptorHeap>> temp;
		std::swap(m_AvailableHeaps, temp);

		for (ComPtr<ID3D12DescriptorHeap> pooledHeap : m_HeapPool)
			m_AvailableHeaps.push(pooledHeap);
		m_CurrDescriptorHeap.Reset();
		m_GraphicsHandleCache.ClearCache();
		m_ComputeHandleCache.ClearCache();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
	{
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}

	void DynamicDescriptorHeap::ParseGraphicsRootSignature(const RootSignature& rootSig)
	{
		m_GraphicsHandleCache.ParseRootSignature(m_Type, rootSig);
	}

	void DynamicDescriptorHeap::ParseComputeRootSignature(const RootSignature& rootSig)
	{
		m_ComputeHandleCache.ParseRootSignature(m_Type, rootSig);
	}

	void DynamicDescriptorHeap::StageGraphicsDescriptorHandles(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle)
	{
		m_GraphicsHandleCache.StageDescriptorHandles(rootParamIdx, offset, numDescriptors, m_DescriptorSize, startCPUHandle);
	}

	void DynamicDescriptorHeap::StageComputeDescriptorHandles(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle)
	{
		m_ComputeHandleCache.StageDescriptorHandles(rootParamIdx, offset, numDescriptors, m_DescriptorSize, startCPUHandle);
	}

	bool DynamicDescriptorHeap::HasSpace(Uint32 numDescriptors)
	{
		return (m_CurrDescriptorHeap != nullptr && m_NumFreeHandles >= numDescriptors * m_DescriptorSize);
	}

	ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::GetHeap()
	{
		if (m_CurrDescriptorHeap == nullptr) {
			m_CurrDescriptorHeap = RequestDescriptorHeap();
			m_CurrHandle = DescriptorHandle(m_CurrDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
				m_CurrDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			m_NumFreeHandles = MaxNumDescriptorsPerHeap;
		}
		return m_CurrDescriptorHeap;
	}

	void DynamicDescriptorHeap::UnbindAllValid()
	{
		m_GraphicsHandleCache.UnbindAllValid();
		m_ComputeHandleCache.UnbindAllValid();
	}

	void DynamicDescriptorHeap::CommitStagedDescriptorTables(DescriptorHandleCache& handleCache, CommitFunc setFunc)
	{

		Uint32 numDescriptorsToCommit = handleCache.ComputeStaleDescriptors();

		if (numDescriptorsToCommit <= 0)
			return;
		// not enough space in the GPU visible descriptor heap
		if (!HasSpace(numDescriptorsToCommit)) {
			// retire heap too
			UnbindAllValid();
			numDescriptorsToCommit = handleCache.ComputeStaleDescriptors();
		}

		m_OwningContext.SetDescriptorHeap(m_Type, GetHeap());
		handleCache.CommitStagedDescriptorTables(m_Type, m_DescriptorSize, m_Device, m_OwningContext, m_CurrHandle, setFunc);
		m_CurrHandle += numDescriptorsToCommit * m_DescriptorSize;
		m_NumFreeHandles -= numDescriptorsToCommit;
	}

	void DynamicDescriptorHeap::DescriptorHandleCache::CommitStagedDescriptorTables(D3D12_DESCRIPTOR_HEAP_TYPE type, Uint32 descriptorSize,
		RenderDevice* device, CommandContext& context, DescriptorHandle currHandle, CommitFunc setFunc)
	{
		DWORD rootIndex;
		while (_BitScanForward(&rootIndex, m_StaleDescriptorBitMask)) {
			m_StaleDescriptorBitMask ^= (1 << rootIndex);

			Uint32 numSrcDescriptors = m_DescriptorTableCache[rootIndex].NumDescriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE* pSrcHandles = m_DescriptorTableCache[rootIndex].RootDescriptor;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRange[] = { currHandle.GetCPUHandle() };
			Uint32						pDestDescriptorRangeSize[] = { numSrcDescriptors };

			device->GetD3DDevice()->CopyDescriptors(1, pDestDescriptorRange, pDestDescriptorRangeSize,
				numSrcDescriptors, pSrcHandles, nullptr, type);

			setFunc(context.GetCommandList(), rootIndex, currHandle.GetGPUHandle());
			currHandle += (numSrcDescriptors * descriptorSize);
		}
	}



	ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::RequestDescriptorHeap()
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		if (!m_AvailableHeaps.empty()) {
			descriptorHeap = m_AvailableHeaps.front();
			m_AvailableHeaps.pop();
		}
		else {
			descriptorHeap = CreateDescriptorHeap();
			m_HeapPool.push_back(descriptorHeap);
		}
		return descriptorHeap;
	}
	ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = m_Type;
		desc.NumDescriptors = MaxNumDescriptorsPerHeap;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		auto hr = m_Device->GetD3DDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating dynamic descriptor heap (shader visible");
		return descriptorHeap;
	}

	void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid()
	{
		m_StaleDescriptorBitMask = 0;

		DWORD tableBitMask = m_DescriptorTableBitMask; // copy of what root indexes have descriptor tables
		DWORD rootIndex;
		while (_BitScanForward(&rootIndex, tableBitMask)) {
			tableBitMask ^= (1 << rootIndex); // removes this root index from bit mask

			m_StaleDescriptorBitMask |= (1 << rootIndex);
		}
	}

	Uint32 DynamicDescriptorHeap::DescriptorHandleCache::ComputeStaleDescriptors() const
	{
		Uint32 numStaleDescriptors = 0;
		DWORD i;
		// copy of the bitmask
		DWORD staleDescriptorBitMask = m_StaleDescriptorBitMask;

		// for each bit that is root param that has been set in the table cache denoted by (1)
		while (_BitScanForward(&i, staleDescriptorBitMask)) {
			// add # of descriptors in that table slot to the total
			numStaleDescriptors += m_DescriptorTableCache[i].NumDescriptors;
			// clear that (1) from the temp bitmask using a bitwise XOR (returns 1 if bits aren't similar)
			staleDescriptorBitMask ^= (1 << i);
		}
		return numStaleDescriptors;
	}

	void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE type, const RootSignature& rootSig)
	{
		DWORD i;
		m_DescriptorTableBitMask = type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ?
			rootSig.m_DescriptorTableBitMask : rootSig.m_SamplerBitMask;
		DWORD descriptorTableBitMask = m_DescriptorTableBitMask;
		Uint32 offset = 0;

		// for each parameter in the root signature that is a descriptor table
		while (_BitScanForward(&i, descriptorTableBitMask)) {
			// retrieve number of descriptors in that table
			Uint32 numDescriptors = rootSig.m_DescriptorTableSize[i];

			// setup cache for later usage
			DescriptorTableCache& tableCache = m_DescriptorTableCache[i];
			tableCache.NumDescriptors = numDescriptors;
			tableCache.RootDescriptor = m_DescriptorHandleCache + offset;

			// moves offset ptr to next descriptor table in the handle cache
			offset += numDescriptors;

			// clears bit from the (temp) bit mask
			descriptorTableBitMask ^= (1 << i);
		}

		KRS_CORE_ASSERT(offset <= DynamicDescriptorHeap::MaxNumDescriptorsPerHeap, "offset exceeds max number of descriptors per heap");
	}

	void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles(Uint32 rootParamIdx, Uint32 offset,
		Uint32 numDescriptors, Uint32 descriptorSize, const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle)
	{
		
		//KRS_CORE_ASSERT(offset + numDescriptors <= DynamicDescriptorHeap::MaxNumDescriptorsPerHeap && rootParamIdx < DynamicDescriptorHeap::MaxDescriptorTables, "Issues");
		KRS_CORE_ASSERT(((1 << rootParamIdx) & m_DescriptorTableBitMask) != 0, "Not a valid index");

		// retrieved cached info about the specific descriptor table
		DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootParamIdx];

		// number of descriptors copying cannot exceed the number of expected descriptors in the descriptor table cache
		KRS_CORE_ASSERT(offset + numDescriptors <= descriptorTableCache.NumDescriptors, "Issues");

		D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = descriptorTableCache.RootDescriptor + offset;

		// iterate through each descriptor from the SRC descriptors and copy it over to the destination
		for (Uint32 i = 0; i < numDescriptors; i++) {
			dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(startCPUHandle, i, descriptorSize);
		}
		// sets root param idx to true (1) in the descriptor table to know this root param idx has a staged descriptor table
		m_StaleDescriptorBitMask |= (1 << rootParamIdx);
	}



}


