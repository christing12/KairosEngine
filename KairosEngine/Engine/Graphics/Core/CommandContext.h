#pragma once

#include "LinearAllocator.h"
#include "DynamicDescriptorHeap.h"

namespace Kairos {
	class RenderDevice;
	class GraphicsContext;
	class RootSignature;
	class PipelineStateObject;

	class CommandContext : public std::enable_shared_from_this<CommandContext> {
	public:
		friend class CommandQueue;
		friend class RenderDevice;

		CommandContext(RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE type);
		~CommandContext();

		ID3D12GraphicsCommandList* GetCommandList() { return m_dCommandList.Get(); }
		D3D12_COMMAND_LIST_TYPE GetType() { return m_Type; }
		ID3D12CommandAllocator* GetAllocator() { return m_CurrCommandAllocator.Get(); }

		void CopyResource(ID3D12Resource* src, ID3D12Resource* dest);
		void TransitionResource(class Resource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void FlushResourceBarriers();

		static void InitBuffer(class RenderDevice* pDevice, Resource& resource, CPVoid data, size_t numBytes, size_t offset = 0);
		void WriteBuffer(Resource& dest, CPVoid data, size_t sizeInBytes, size_t destOffset = 0);
		static void InitTexture(RenderDevice* pDevice, Resource& resource, Uint32 numSubResources, const D3D12_SUBRESOURCE_DATA subresources[]);

		void Reset(Ref<class CommandQueue> commandQueue);
		Uint64 Submit(bool waitForCompletion);
		GraphicsContext& GetGraphicsContext();

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	protected:
		void BindDescriptorHeaps();

		LinearAllocator::Allocation RequestUploadMemory(size_t sizeInBytes, size_t alignment = 256);

		RenderDevice* m_Device = nullptr;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_dCommandList = nullptr;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		D3D12_PRIMITIVE_TOPOLOGY m_Topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CurrCommandAllocator = nullptr;
		LinearAllocator m_CPUAllocator;

		Uint32 m_BarriersToFlush = 0;
		D3D12_RESOURCE_BARRIER m_ResourceBarriers[16];

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		DynamicDescriptorHeap m_ViewDescriptorHeap;
		DynamicDescriptorHeap m_SamplerDescriptorHeap;

		ID3D12RootSignature* m_CurrRootSignature = nullptr;
		ID3D12PipelineState* m_CurrPSO = nullptr;
	};

	class GraphicsContext : public CommandContext {
	public:
		void ClearColor(class Texture& target);
		void ClearDepth(class Texture& target, D3D12_CLEAR_FLAGS clearFlags, float depth, Uint8 stencil);

		void SetViewportScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect);
		void SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[]);
		void SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[],
			const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);
		void SetConstants(Uint32 rootIdx, Uint32 numConstants, CPVoid pConstants);

		void SetTopology(D3D_PRIMITIVE_TOPOLOGY);
		void SetShaderConstants(Uint32 rootIndex, Uint32 numConstants, CPVoid data);
		void SetRootConstantBuffer(Uint32 rootIndex, D3D12_GPU_VIRTUAL_ADDRESS gpuAddress);
		void SetDynamicCBV(Uint32 rootIndex, Uint32 size, CPVoid data);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& bufferView);
		void SetVertexBuffers(Uint32 offset, Uint32 numBuffers, const D3D12_VERTEX_BUFFER_VIEW& bufferView);

		void DrawInstanced(Uint32 numVerticesPerInstance, Uint32 numInstances,
			Uint32 vertexOffset, Uint32 startInstanceLoc);
		void DrawIndexedInstance(Uint32 instIdxCnt, Uint32 numInstance, Uint32 startIndex, Uint32 vertLoc, Uint32 startInstancePos);

		void Draw(Uint32 numVertices, Uint32 vertexOffset);
		void DrawIndexed(Uint32 numIndices, Uint32 indexOffset, int baseVertexLocation);

		void SetRootSignature(const RootSignature& rootSig);
		void SetPipelineState(const PipelineStateObject& PSO);
		void SetDynamicDescriptors(Uint32 rootIndex, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle);
		void SetDynamicSamplers(Uint32 rootIndex, Uint32 offset, Uint32 count, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle);
	private:
	};

	class ComputeContext : public CommandContext {

	};
}