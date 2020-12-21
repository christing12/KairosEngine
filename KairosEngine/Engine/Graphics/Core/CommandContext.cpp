#include "krspch.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "CommandQueue.h"
#include "Resource.h"
#include "RenderDevice.h"
#include "Texture.h"

namespace Kairos {



    void GraphicsContext::ClearColor(Texture& target)
    {
        float temp[4] = { 0.f, 0.f, 0.f, 1.f };
        m_dCommandList->ClearRenderTargetView(target.GetRTV(), (FLOAT*)temp, 0, nullptr);
    }

    void GraphicsContext::ClearDepth(Texture& target, D3D12_CLEAR_FLAGS clearFlags, float depth, Uint8 stencil)
    {
        m_dCommandList->ClearDepthStencilView(target.GetDSV(), clearFlags, depth, stencil, 0, nullptr);
    }

    void GraphicsContext::SetRootSignature(const RootSignature& rootSignature)
    {
        m_CurrRootSignature = rootSignature.GetD3DRootSignature();
        m_dCommandList->SetGraphicsRootSignature(m_CurrRootSignature);
        m_ViewDescriptorHeap.ParseRootSignature(rootSignature);
        m_SamplerDescriptorHeap.ParseRootSignature(rootSignature);
    }

    void GraphicsContext::SetPipelineState(const PipelineStateObject& PSO)
    {
        if (PSO.GetPipelineState() == nullptr)
            return;
        m_CurrPSO = PSO.GetPipelineState();
        m_dCommandList->SetPipelineState(m_CurrPSO);
    }

    void GraphicsContext::SetViewportScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect)
    {
        m_dCommandList->RSSetViewports(1, &viewport);
        m_dCommandList->RSSetScissorRects(1, &rect);
    }
    void GraphicsContext::SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[])
    {
        m_dCommandList->OMSetRenderTargets(numRTV, rtvHandles, FALSE, nullptr);
    }
    void GraphicsContext::SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[], const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
    {
        m_dCommandList->OMSetRenderTargets(numRTV, rtvHandles, FALSE, &dsvHandle);
    }

    void GraphicsContext::SetTopology(D3D_PRIMITIVE_TOPOLOGY topology)
    {
        m_dCommandList->IASetPrimitiveTopology(topology);
    }
    void GraphicsContext::SetShaderConstants(Uint32 rootIndex, Uint32 numConstants, CPVoid data)
    {
        m_dCommandList->SetGraphicsRoot32BitConstants(rootIndex, numConstants, data, 0);
    }

    void GraphicsContext::SetRootConstantBuffer(Uint32 rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAddress)
    {
        m_ViewDescriptorHeap.StageCBV(rootIndex, gpuAddress);
    }

    void GraphicsContext::SetDynamicCBV(Uint32 rootIndex, Uint32 size, CPVoid data)
    {
        LinearAllocator::Allocation dynAlloc = RequestUploadMemory(size, 256); // entire resource already mapped
        memcpy(dynAlloc.CPU, data, size);
        m_dCommandList->SetGraphicsRootConstantBufferView(rootIndex, dynAlloc.GPU);
    }

    void GraphicsContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& bufferView)
    {
        m_dCommandList->IASetIndexBuffer(&bufferView);
    }

    void GraphicsContext::SetVertexBuffers(Uint32 offset, Uint32 numBuffers, const D3D12_VERTEX_BUFFER_VIEW& bufferView)
    {
        m_dCommandList->IASetVertexBuffers(offset, numBuffers, &bufferView);
    }

    void GraphicsContext::DrawInstanced(Uint32 numVerticesPerInstance, Uint32 numInstances, Uint32 vertexOffset, Uint32 startInstanceLoc)
    {
        FlushResourceBarriers();
        m_ViewDescriptorHeap.CommitStagedDescriptors();
        m_SamplerDescriptorHeap.CommitStagedDescriptors();
        m_dCommandList->DrawInstanced(numVerticesPerInstance, numInstances, vertexOffset, startInstanceLoc);
    }

    void GraphicsContext::DrawIndexedInstance(Uint32 instIdxCnt, Uint32 numInstance, Uint32 startIndex, Uint32 vertLoc, Uint32 startInstancePos)
    {
        FlushResourceBarriers();
        m_ViewDescriptorHeap.CommitStagedDescriptors();
        m_SamplerDescriptorHeap.CommitStagedDescriptors();
        m_dCommandList->DrawIndexedInstanced(instIdxCnt, numInstance, startIndex, vertLoc, startInstancePos);
    }

    void GraphicsContext::Draw(Uint32 numVertices, Uint32 vertexOffset)
    {
        DrawInstanced(numVertices, 1, vertexOffset, 0);
    }

    void GraphicsContext::DrawIndexed(Uint32 numIndices, Uint32 indexOffset, int baseVertexLocation)
    {
        DrawIndexedInstance(numIndices, 1, indexOffset, baseVertexLocation, 0);
    }

    void GraphicsContext::SetDynamicDescriptors(Uint32 rootIndex, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle)
    {
        m_ViewDescriptorHeap.StageDescriptorTable(rootIndex, offset, numDescriptors, startHandle);
    }

    void GraphicsContext::SetDynamicSamplers(Uint32 rootIndex, Uint32 offset, Uint32 count, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle)
    {
        m_SamplerDescriptorHeap.StageDescriptorTable(rootIndex, offset, count, startHandle);
    }

    void GraphicsContext::SetConstants(Uint32 rootIdx, Uint32 numConstants, CPVoid pConstants) {
        m_dCommandList->SetGraphicsRoot32BitConstants(rootIdx, numConstants, pConstants, 0);
    }

    CommandContext::CommandContext(RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE commandType)
        : m_Device(pDevice)
        , m_Type(commandType)
        , m_CPUAllocator(pDevice, CPU_WRITABLE)
        , m_ViewDescriptorHeap(pDevice, *this, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
        , m_SamplerDescriptorHeap(pDevice, *this, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)
    {
        FlushResourceBarriers();
        pDevice->CreateCommandList(commandType, &m_dCommandList, &m_CurrCommandAllocator);
    }

    CommandContext::~CommandContext()
    {
        if (m_CurrCommandAllocator != nullptr) {
            std::cout << " WT F < " << std::endl;
        }
    }

    void CommandContext::CopyResource(ID3D12Resource* src, ID3D12Resource* dest)
    {
    }

    void CommandContext::TransitionResource(Resource& pResource, D3D12_RESOURCE_STATES newState, bool flushImmediate)
    {
        D3D12_RESOURCE_STATES oldState = pResource.GetCurrState();
        if (oldState != newState) {
            D3D12_RESOURCE_BARRIER& barrier = m_ResourceBarriers[m_BarriersToFlush++];
            barrier = CD3DX12_RESOURCE_BARRIER::Transition
            (pResource.GetResource(), pResource.GetCurrState(), newState);
            pResource.m_CurrState = newState;
        }

        if (flushImmediate || m_BarriersToFlush == 16) {
            FlushResourceBarriers();
        }
    }

    void CommandContext::FlushResourceBarriers()
    {
        if (m_BarriersToFlush > 0)
        {
            m_dCommandList->ResourceBarrier(m_BarriersToFlush, m_ResourceBarriers);
            m_BarriersToFlush = 0;
        }
    }

    void CommandContext::InitBuffer(RenderDevice* pDevice, Resource& resource, CPVoid data, size_t numBytes, size_t offset)
    {
        CommandContext& context = pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
        LinearAllocator::Allocation dynAlloc = context.RequestUploadMemory(numBytes);

        memcpy(dynAlloc.CPU, data, numBytes);
        if (resource.GetCurrState() != D3D12_RESOURCE_STATE_COPY_DEST)
            context.TransitionResource(resource, D3D12_RESOURCE_STATE_COPY_DEST, true);

        // copies from upload buffer (allocation) to dst resource GPU only (resource)
        context.m_dCommandList->CopyBufferRegion(resource.GetResource(), offset,
            dynAlloc.UploadResource.Get(), 0, numBytes);
        context.TransitionResource(resource, D3D12_RESOURCE_STATE_GENERIC_READ, true);
        context.Submit(true);
    }

    void CommandContext::InitTexture(RenderDevice* pDevice, Resource& resource, Uint32 numSubresources, D3D12_SUBRESOURCE_DATA subresources[])
    {
        Uint64 reqSize = GetRequiredIntermediateSize(resource.GetResource(), 0, numSubresources);

        CommandContext& context = pDevice->AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
        LinearAllocator::Allocation dynAlloc = context.RequestUploadMemory(reqSize);

        UpdateSubresources(context.m_dCommandList.Get(), resource.GetResource(), dynAlloc.UploadResource.Get(),
            0, 0, numSubresources, subresources);

        context.TransitionResource(resource, D3D12_RESOURCE_STATE_GENERIC_READ);
        context.Submit(true);
    }

    void CommandContext::Reset(Ref<class CommandQueue> commandQueue)
    {
        KRS_CORE_ASSERT(m_dCommandList != nullptr && m_CurrCommandAllocator == nullptr, "Not a previously freed context");
        m_Device->GetCommandQueue(m_Type)->GetFreeAllocator(&m_CurrCommandAllocator);
        auto hr = m_dCommandList->Reset(m_CurrCommandAllocator.Get(), nullptr);
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Success");

        m_CurrPSO = nullptr;
        m_CurrRootSignature = nullptr;
        m_CPUAllocator.Reset();
        m_ViewDescriptorHeap.Reset();

        m_BarriersToFlush = 0;
        BindDescriptorHeaps();
    }

    Uint64 CommandContext::Submit(bool waitForCompletion)
    {
        Ref<CommandQueue> commandQueue = m_Device->GetCommandQueue(m_Type);
        Uint64 fenceValue = commandQueue->ExecuteCommandList(this);
        commandQueue->DiscardAllocator(fenceValue, m_CurrCommandAllocator);
        m_CurrCommandAllocator = nullptr;

        if (waitForCompletion) {
            commandQueue->WaitForGPU(fenceValue);
        }
        //m_CPUAllocator.Reset();
        m_Device->FreeCommandContext(this);

        KRS_CORE_ASSERT(m_CurrCommandAllocator == nullptr, "ISSSEUIOWFJW");
        return fenceValue;
    }

    GraphicsContext& CommandContext::GetGraphicsContext()
    {
        KRS_CORE_ASSERT(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "WRONG TYPE OF CONTEXT");
        return reinterpret_cast<GraphicsContext&>(*this);
    }
    void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ComPtr<ID3D12DescriptorHeap> heap)
    {

        if (m_CurrDescriptorHeaps[type] != heap) {
            m_CurrDescriptorHeaps[type] = heap;
            BindDescriptorHeaps();
        }
    }
    void CommandContext::BindDescriptorHeaps()
    {
        Uint32 numDescriptorHeaps = 0;
        ID3D12DescriptorHeap* BindableDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
        for (Uint32 i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
            ComPtr<ID3D12DescriptorHeap> heap = m_CurrDescriptorHeaps[i];
            if (heap != nullptr) {
                BindableDescriptorHeaps[numDescriptorHeaps++] = heap.Get();
            }
        }
        if (numDescriptorHeaps)
            m_dCommandList->SetDescriptorHeaps(numDescriptorHeaps, BindableDescriptorHeaps);
    }
    LinearAllocator::Allocation CommandContext::RequestUploadMemory(size_t sizeInBytes, size_t alignment)
    {
        return m_CPUAllocator.Allocate(sizeInBytes, alignment);
    }
}