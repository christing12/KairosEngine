#include "krspch.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "CommandQueue.h"
#include "RenderDevice.h"
#include "BufferManager.h"
#include "Texture.h"
#include "PipelineStateManager.h"
#include "Systems/Engine.h"

// TODO: REMOVE THIS ITS TEMP
extern Kairos::Engine* g_Engine;

namespace Kairos {
    CommandContext::CommandContext(RenderDevice* pDevice, CommandType type)
        : m_Device(pDevice)
        , m_Type(type)
        , m_CPUAllocator(pDevice, 256)
    {
        FlushResourceBarriers();

    pDevice->GetCommandQueue(type).GetFreeAllocator(&m_CurrCommandAllocator);
        KRS_CORE_ASSERT(m_CurrCommandAllocator != nullptr, "Issue with getting a non nullptr command allocator");
        auto hr = pDevice->D3DDevice()->CreateCommandList(1, D3DCommandType(type), 
            m_CurrCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_dCommandList));
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating a new D3D Command List");
    }


    CommandContext::~CommandContext() {
        if (m_CurrCommandAllocator != nullptr) {
            std::cout << " WT F < " << std::endl;
        }
    }


    void CommandContext::SetScene(Scene* scene)
    {
        m_Scene = scene;
    }

    void CommandContext::TransitionResource(GPUResource& resource, ResourceState newState, bool flushImmediate)
    {
        
        D3D12_RESOURCE_STATES oldState = D3DResourceStates(resource.ResourceState());
        if (resource.ResourceState() != newState) {
            D3D12_RESOURCE_BARRIER& barrier = m_ResourceBarriers[m_BarriersToFlush++];
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.D3DResource(), oldState, D3DResourceStates(newState));
            resource.SetResourceState(newState);
        }

        if (flushImmediate || m_BarriersToFlush == DEFAULT_MAX_BARRIERS) {
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


    void CommandContext::CopyBufferRegion(RenderHandle dest, Uint32 destOffset, CPVoid data, Uint32 sizeInBytes)
    {
        DynAlloc dynAlloc = RequestUploadMemory(sizeInBytes);
        memcpy(dynAlloc.CPUPtr, data, sizeInBytes);

        Buffer* buffer = m_Device->m_BufferManager->GetBuffer(dest);

        KRS_CORE_ASSERT(buffer != nullptr, "Buffer trying to copy to is a nullptr")
        m_dCommandList->CopyBufferRegion(buffer->D3DResource(),
            buffer->Offset() + destOffset, dynAlloc.Resource.Get(), 0, sizeInBytes);
    }

    void CommandContext::CopyBufferRegion(Buffer& buffer, Uint32 destOffset, CPVoid data, Uint32 sizeInBytes)
    {
        DynAlloc dynAlloc = RequestUploadMemory(sizeInBytes);
        memcpy(dynAlloc.CPUPtr, data, sizeInBytes);

        m_dCommandList->CopyBufferRegion(buffer.D3DResource(),
            buffer.Offset() + destOffset, dynAlloc.Resource.Get(), 0, sizeInBytes);
    }

    void CommandContext::CopyResource(GPUResource& dest, GPUResource& src)
    {
        m_dCommandList->CopyResource(dest.D3DResource(), src.D3DResource());
    }

    //TODO: Make it 
    void CommandContext::CopyTextureRegion(Texture* dest, Texture* src, Uint32 x, Uint32 y, Uint32 z)
    {

        KRS_CORE_ASSERT(dest->Depth() == src->Depth(), "Incompatiable number of array slices (depth)");
        TransitionResource(*dest, ResourceState::CopyDest);
        TransitionResource(*src, ResourceState::CopySrc, true);
        for (Uint32 arraySlice = 0; arraySlice < dest->Depth(); ++arraySlice)
        {
            const UINT subresourceIndex = D3D12CalcSubresource(0, arraySlice, 0, dest->MipLevels() , 6);
            m_dCommandList->CopyTextureRegion(
                &CD3DX12_TEXTURE_COPY_LOCATION(dest->D3DResource(), subresourceIndex),
                x, y, z,
                &CD3DX12_TEXTURE_COPY_LOCATION(src->D3DResource(), subresourceIndex),
                nullptr
            );
        }
    }


    void CommandContext::InitTexture(Texture& texture, Uint32 numSubresources, const D3D12_SUBRESOURCE_DATA subresources[])
    {
        Uint64 reqSize = GetRequiredIntermediateSize(texture.D3DResource(), 0, numSubresources);
        DynAlloc dynAlloc = RequestUploadMemory(reqSize);

        UpdateSubresources(m_dCommandList.Get(), texture.D3DResource(), dynAlloc.Resource.Get(),
            0, 0, numSubresources, subresources);
    }



    void CommandContext::Reset(CommandQueue* commandQueue)
    {
        KRS_CORE_ASSERT(m_dCommandList != nullptr && m_CurrCommandAllocator == nullptr, "Not a previously freed context");

       m_Device->GetCommandQueue(m_Type).GetFreeAllocator(&m_CurrCommandAllocator);

        auto hr = m_dCommandList->Reset(m_CurrCommandAllocator.Get(), nullptr);
        KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with Resetting Current Command Allocator");

        m_CurrPSO = nullptr;
        m_CurrRootSig = nullptr;
        m_BarriersToFlush = 0;
        BindDescriptorHeaps();
    }





    Uint64 CommandContext::Submit(bool waitForCompletion)
    {
        FlushResourceBarriers();

        CommandQueue& commandQueue = m_Device->GetCommandQueue(m_Type);
        Uint64 fenceValue = commandQueue.ExecuteCommands(*this);
        commandQueue.DiscardAllocator(fenceValue, m_CurrCommandAllocator);
        m_CurrCommandAllocator = nullptr;


        if (waitForCompletion) {
            commandQueue.WaitForGPU(fenceValue);
        }
        m_CPUAllocator.CleanupPages(fenceValue);
        m_Device->FreeCommandContext(*this);

        KRS_CORE_ASSERT(m_CurrCommandAllocator == nullptr, "ISSSEUIOWFJW");
        return fenceValue;
    }


    GraphicsContext& CommandContext::GetGraphicsContext()
    {
        KRS_CORE_ASSERT(m_Type != CommandType::Compute, "WRONG TYPE OF CONTEXT");
        return reinterpret_cast<GraphicsContext&>(*this);
    }


    ComputeContext& CommandContext::GetComputeContext()
    {
        KRS_CORE_ASSERT(m_Type != CommandType::Graphics, "WRONG TYPE OF CONTEXT");
        return reinterpret_cast<ComputeContext&>(*this);
    }


    void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ComPtr<ID3D12DescriptorHeap> heap)
    {
        if (m_CurrDescriptorHeaps[type] != heap) {
            m_CurrDescriptorHeaps[type] = heap;
            BindDescriptorHeaps();
        }
    }

    /* COMMAND CONTEXT HELPERS */

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


    DynAlloc CommandContext::RequestUploadMemory(size_t sizeInBytes, size_t alignment)
    {
        return m_CPUAllocator.Allocate(sizeInBytes, alignment);
    }




    /* ------------------- GRAPHICS CONTEXT START -------------------------------- */

    void GraphicsContext::ClearColor(Texture& target)
    {
        float temp[4] = { 0.25f, 0.25f, 0.25f, 1.f };
        m_dCommandList->ClearRenderTargetView(target.GetRTVDescriptor(0).CPUHandle(), &temp[0], 0, nullptr);
    }

    void GraphicsContext::ClearDepth(Texture& target, D3D12_CLEAR_FLAGS clearFlags, float depth, Uint8 stencil)
    {
        m_dCommandList->ClearDepthStencilView(target.GetDSVDescriptor().CPUHandle(), clearFlags, depth, stencil, 0, nullptr);
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

    void GraphicsContext::SetDynamicCBV(Uint32 rootIndex, Uint32 size, CPVoid data)
    {
        DynAlloc dynAlloc = RequestUploadMemory(size, 256);
        memcpy(dynAlloc.CPUPtr, data, size);
        m_dCommandList->SetGraphicsRootConstantBufferView(rootIndex, dynAlloc.gpuAdress);
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
        m_dCommandList->DrawInstanced(numVerticesPerInstance, numInstances, vertexOffset, startInstanceLoc);
    }

    void GraphicsContext::DrawIndexedInstance(Uint32 instIdxCnt, Uint32 numInstance, Uint32 startIndex, Uint32 vertLoc, Uint32 startInstancePos)
    {
        FlushResourceBarriers();
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




    void GraphicsContext::SetConstants(Uint32 rootIdx, Uint32 numConstants, CPVoid pConstants) {
        auto index = m_CurrRootSig->GetParameterIndex({ rootIdx, 0, ShaderRegister::ConstantBuffer });
        m_dCommandList->SetGraphicsRoot32BitConstants(index->index, numConstants, pConstants, 0);
    }
 

    void GraphicsContext::SetRootSignature(RootSignature* rootSignature)
    {
        m_CurrRootSig = rootSignature;
        m_dCommandList->SetGraphicsRootSignature(m_CurrRootSig->D3DRootSignature());
    }



    void GraphicsContext::SetPipelineState(const std::string name)
    {
        Kairos::PSOQueryResult result = g_Engine->GetRenderBackend()->PipelineManager()->GetPipelineState(name);


        m_CurrPSO = result.Pipeline;
        m_dCommandList->SetPipelineState(m_CurrPSO->GetPipelineState());

        SetTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        SetRootSignature(result.Pipeline->GetSignature());

        BindCommonResources(result.Pipeline->GetSignature());
    }

    void GraphicsContext::BindCommonResources(RootSignature* rootSig)
    {
        Uint32 commonParamOffset = rootSig->NumParameters() - g_Engine->GetRenderBackend()->PipelineManager()->NumCommonRootParamters();
        
        Descriptor desc = m_Device->m_OnlineCBV.GetStart();
        D3D12_GPU_DESCRIPTOR_HANDLE SRVRangeStart = m_Device->m_OnlineCBV.RangeStart(OnlineDescriptorHeap::Range::SRV).GPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE UARangeStart = m_Device->m_OnlineCBV.RangeStart(OnlineDescriptorHeap::Range::UAV).GPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE SamplerRangeStart = m_Device->m_OnlineSamplerHeap.RangeStart(static_cast<OnlineDescriptorHeap::Range>(0)).GPUHandle();


        m_dCommandList->SetGraphicsRootDescriptorTable(3 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(4 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(5 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(6 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(7 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(8 + commonParamOffset, SRVRangeStart);

        m_dCommandList->SetGraphicsRootDescriptorTable(9 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(10 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(11 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(12 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(13 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(14 + commonParamOffset, UARangeStart);
        m_dCommandList->SetGraphicsRootDescriptorTable(15 + commonParamOffset, UARangeStart);

        m_dCommandList->SetGraphicsRootDescriptorTable(16 + commonParamOffset, SamplerRangeStart);

        m_dCommandList->SetGraphicsRootConstantBufferView(commonParamOffset, m_Device->GlobalBuffer()->GPUVirtualAdress());
        m_dCommandList->SetGraphicsRootConstantBufferView(commonParamOffset + 1, m_Device->PerFrameBuffer()->GPUVirtualAdress());
    }

    void GraphicsContext::BindPassConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS address)
    {
        Uint32 commonParamOffset = m_CurrRootSig->NumParameters() - g_Engine->GetRenderBackend()->PipelineManager()->NumCommonRootParamters();
        m_dCommandList->SetGraphicsRootConstantBufferView(2 + commonParamOffset, address);

    }

    void GraphicsContext::BindBuffer(Buffer& buffer, Uint16 shaderRegister, Uint16 registerSpace, ShaderRegister registerType)
    {
        const RootSignature* signature = m_CurrRootSig;
        auto index = signature->GetParameterIndex({ shaderRegister, registerSpace, registerType });
        

        switch (registerType)
        {
        case ShaderRegister::ConstantBuffer: m_dCommandList->SetGraphicsRootConstantBufferView(index->index, buffer.GPUVirtualAdress()); break;
        case ShaderRegister::ShaderResource: m_dCommandList->SetGraphicsRootDescriptorTable(index->index, buffer.GetSRDescriptor().GPUHandle()); break;
    //    case ShaderRegister::UnorderedAcces: m_dCommandList->SetGraphicsRootDescriptorTable(index->index, buffer.GetSRDescriptor()->GPUHandle()); break;
     //   case ShaderRegister::Sampler: assert_format(false, "Incompatible register type");
        }
    }


    //void GraphicsContext::BindBuffer(const Buffer* buffer, Uint16 shaderRegister, Uint16 registerSpace, ShaderRegister registerType)
    //{
    //    auto index = m_CurrRootSig->GetParameterIndex({ shaderRegister, registerSpace, registerType });
    //    KRS_CORE_ASSERT(index, "parameter doesn't exist");

    //    //switch (registerType)
    //    //{
    //    //case ShaderRegister::ShaderResource: m_dCommandList->SetGraphicsRootDescriptorTable(buffer - )
    //    //}
    //}


    // ------------------------------ COMPUTE ------------------------------------- //


    void ComputeContext::SetRootSignature(RootSignature* RootSig)
    {
        m_CurrRootSig = RootSig;

        m_dCommandList->SetComputeRootSignature(m_CurrRootSig->D3DRootSignature());
    }
    void ComputeContext::SetConstantArray(Uint32 RootIndex, Uint32 NumConstants, const void* pConstants)
    {
        m_dCommandList->SetComputeRoot32BitConstants(RootIndex, NumConstants, pConstants, 0);
    }
    void ComputeContext::SetDescriptorTable(Uint32 rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle)
    {
        m_dCommandList->SetComputeRootDescriptorTable(rootIndex, startGPUHandle);
    }

    void ComputeContext::SetRootConstants(Uint32 rootIdx, Uint32 space, Uint32 numConstants, CPVoid data)
    {
        auto index = m_CurrRootSig->GetParameterIndex({ rootIdx, space, ShaderRegister::ConstantBuffer });
        m_dCommandList->SetComputeRoot32BitConstants(index.value().index, numConstants, data, 0);
    }

    void ComputeContext::Dispatch(Uint32 groupCountX, Uint32 groupCountY, Uint32 groupCountZ)
    {
        FlushResourceBarriers();;

        m_dCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
    }


    void ComputeContext::SetPipelineState(const std::string name)
    {
        Kairos::PSOQueryResult result = g_Engine->GetRenderBackend()->PipelineManager()->GetPipelineState(name);

        m_CurrPSO = result.Pipeline;
        m_dCommandList->SetPipelineState(m_CurrPSO->GetPipelineState());
        SetRootSignature(result.Pipeline->GetSignature());

        BindCommonResources(result.Pipeline->GetSignature());
    }


    void ComputeContext::BindCommonResources(RootSignature* rootSig)
    {
        Uint32 commonParamOffset = rootSig->NumParameters() - g_Engine->GetRenderBackend()->PipelineManager()->NumCommonRootParamters();

        Descriptor desc = m_Device->m_OnlineCBV.GetStart();
        D3D12_GPU_DESCRIPTOR_HANDLE SRVRangeStart = m_Device->m_OnlineCBV.RangeStart(OnlineDescriptorHeap::Range::SRV).GPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE UARangeStart = m_Device->m_OnlineCBV.RangeStart(OnlineDescriptorHeap::Range::UAV).GPUHandle();
        D3D12_GPU_DESCRIPTOR_HANDLE SamplerRangeStart = m_Device->m_OnlineSamplerHeap.RangeStart(static_cast<OnlineDescriptorHeap::Range>(0)).GPUHandle();


        m_dCommandList->SetComputeRootDescriptorTable(3 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(4 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(5 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(6 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(7 + commonParamOffset, SRVRangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(8 + commonParamOffset, SRVRangeStart);

        m_dCommandList->SetComputeRootDescriptorTable(9 + commonParamOffset,  UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(10 + commonParamOffset, UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(11 + commonParamOffset, UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(12 + commonParamOffset, UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(13 + commonParamOffset, UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(14 + commonParamOffset, UARangeStart);
        m_dCommandList->SetComputeRootDescriptorTable(15 + commonParamOffset, UARangeStart);

        m_dCommandList->SetComputeRootDescriptorTable(16 + commonParamOffset, SamplerRangeStart);

        // global CBV
        m_dCommandList->SetComputeRootConstantBufferView(commonParamOffset, m_Device->GlobalBuffer()->GPUVirtualAdress());
        // per frame CBV
        m_dCommandList->SetComputeRootConstantBufferView(commonParamOffset + 1, m_Device->PerFrameBuffer()->GPUVirtualAdress());
    }

    void ComputeContext::BindPassConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS address)
    {
        Uint32 commonParamOffset = m_CurrRootSig->NumParameters() - g_Engine->GetRenderBackend()->PipelineManager()->NumCommonRootParamters();
        m_dCommandList->SetComputeRootConstantBufferView(2 + commonParamOffset, address);
    }
}