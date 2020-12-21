#include "krspch.h"
#include "DynamicDescriptorHeap.h"
#include "RenderDevice.h"
#include "RootSignature.h"
#include "CommandContext.h"

namespace Kairos {
	DynamicDescriptorHeap::DynamicDescriptorHeap(RenderDevice* pDevice, CommandContext& context,
		D3D12_DESCRIPTOR_HEAP_TYPE type, Uint32 numDescriptorsInHeap)
		: m_Device(pDevice)
		, m_OwningContext(context)
		, m_Type(type)
		, m_NumDescriptorsPerHeap(numDescriptorsInHeap)
		, m_CurrCPUHandle(D3D12_DEFAULT)
		, m_CurrGPUHandle(D3D12_DEFAULT)
		, m_NumFreeHandles(0)
		, m_StaleDescriptorBitMask(0)
		, m_DescriptorTableBitMask(0)

	{
		// queries for descriptor size in bytes (varies by vendor)
		m_DescriptorSize = m_Device->GetD3DDevice()->GetDescriptorHandleIncrementSize(type);
		m_DescriptorHandleCache = CreateScope<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_NumDescriptorsPerHeap);
	}

	void DynamicDescriptorHeap::Reset()
	{
		std::queue<ComPtr<ID3D12DescriptorHeap>> temp;
		std::swap(m_AvailableHeaps, temp);

		for (ComPtr<ID3D12DescriptorHeap> pooledHeap : m_HeapPool)
			m_AvailableHeaps.push(pooledHeap);
		m_CurrDescriptorHeap.Reset();
		m_CurrCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_CurrGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
		m_DescriptorTableBitMask = 0;
		m_StaleDescriptorBitMask = 0;
		m_NumFreeHandles = 0;
		m_CBVBitMask = 0;



		for (int i = 0; i < MaxDescriptorTables; ++i) {
			m_DescriptorTableCache[i].Reset();
			m_CBVCache[i] = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}
	}

	D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::CopyDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor)
	{
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}

	void DynamicDescriptorHeap::ParseRootSignature(const RootSignature& rootSig)
	{
		DWORD i;
		m_DescriptorTableBitMask = m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ?
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
			tableCache.RootDescriptor = m_DescriptorHandleCache.get() + offset;

			// moves offset ptr to next descriptor table in the handle cache
			offset += numDescriptors;

			// clears bit from the (temp) bit mask
			descriptorTableBitMask ^= (1 << i);
		}

		KRS_CORE_ASSERT(offset <= m_NumDescriptorsPerHeap, "offset exceeds max number of descriptors per heap");

	}

	Uint32 DynamicDescriptorHeap::ComputeStaleDescriptors() const
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

	void DynamicDescriptorHeap::StageDescriptorTable(Uint32 rootParamIdx, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startCPUHandle)
	{
		KRS_CORE_ASSERT(numDescriptors <= m_NumDescriptorsPerHeap && rootParamIdx < MaxDescriptorTables, "Issues");

		// retrieved cached info about the specific descriptor table
		DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootParamIdx];

		// number of descriptors copying cannot exceed the number of expected descriptors in the descriptor table cache
		KRS_CORE_ASSERT(offset + numDescriptors <= descriptorTableCache.NumDescriptors, "Issues");

		D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = descriptorTableCache.RootDescriptor + offset;

		// iterate through each descriptor from the SRC descriptors and copy it over to the destination
		for (Uint32 i = 0; i < numDescriptors; i++) {
			dstDescriptor[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(startCPUHandle, i, m_DescriptorSize);
		}
		// sets root param idx to true (1) in the descriptor table to know this root param idx has a staged descriptor table
		m_StaleDescriptorBitMask |= (1 << rootParamIdx);
	}

	void DynamicDescriptorHeap::StageCBV(Uint32 rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAddress)
	{
		KRS_CORE_ASSERT(rootIndex < MaxDescriptorTables, "index out of bounds");
		m_CBVCache[rootIndex] = gpuAddress;
		m_CBVBitMask |= (1 << rootIndex);
	}

	void DynamicDescriptorHeap::CommitStagedDescriptors()
	{
		CommitStagedDescriptorTables();

		DWORD rootIdx;
		while (_BitScanForward(&rootIdx, m_CBVBitMask)) {
			D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = m_CBVCache[rootIdx];
			m_OwningContext.GetCommandList()->SetGraphicsRootConstantBufferView(rootIdx, gpuAddress);
			m_CBVBitMask ^= (1 << rootIdx);
		}
	}

	void DynamicDescriptorHeap::CommitStagedDescriptorTables()
	{
		Uint32 numDescriptorsToCommit = ComputeStaleDescriptors();

		if (numDescriptorsToCommit <= 0)
			return;
		// not enough space in the GPU visible descriptor heap
		if (!m_CurrDescriptorHeap || m_NumFreeHandles < numDescriptorsToCommit) {
			m_CurrDescriptorHeap = RequestDescriptorHeap();
			m_CurrCPUHandle = m_CurrDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_CurrGPUHandle = m_CurrDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
			m_NumFreeHandles = m_NumDescriptorsPerHeap;

			m_OwningContext.SetDescriptorHeap(m_Type, m_CurrDescriptorHeap);
			m_StaleDescriptorBitMask = m_DescriptorTableBitMask;
		}

		DWORD rootIdx;
		while (_BitScanForward(&rootIdx, m_StaleDescriptorBitMask)) {
			Uint32 numSrcDescriptors = m_DescriptorTableCache[rootIdx].NumDescriptors;
			D3D12_CPU_DESCRIPTOR_HANDLE* pSrcHandles = m_DescriptorTableCache[rootIdx].RootDescriptor;

			D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRange[] = { m_CurrCPUHandle };
			Uint32						pDestDescriptorRangeSize[] = { numSrcDescriptors };

			m_Device->GetD3DDevice()->CopyDescriptors(1, pDestDescriptorRange, pDestDescriptorRangeSize,
				numSrcDescriptors, pSrcHandles, nullptr, m_Type);

			m_OwningContext.GetCommandList()->SetGraphicsRootDescriptorTable(rootIdx, m_CurrGPUHandle);

			m_CurrCPUHandle.Offset(numSrcDescriptors, m_DescriptorSize);
			m_CurrGPUHandle.Offset(numSrcDescriptors, m_DescriptorSize);
			m_NumFreeHandles -= numSrcDescriptors;

			m_StaleDescriptorBitMask ^= (1 << rootIdx);

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
		desc.NumDescriptors = m_NumDescriptorsPerHeap;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		auto hr = m_Device->GetD3DDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
		KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating dynamic descriptor heap (shader visible");
		return descriptorHeap;
	}
}