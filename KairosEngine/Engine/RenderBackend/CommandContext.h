#pragma once

#include "UploadResourceAllocator.h"
#include "DynamicDescriptorHeap.h"

#include "GraphicsTypes.h"

namespace Kairos {
	class RenderDevice;
	class GraphicsContext;
	class RootSignature;
	class PipelineState;
	class ComputeContext;
	class GPUResource;

	class CommandContext : public std::enable_shared_from_this<CommandContext> {
	public:
		friend class CommandQueue;
		friend class RenderDevice;

		CommandContext(RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE type);
		~CommandContext();

		ID3D12GraphicsCommandList* GetCommandList() { return m_dCommandList.Get(); }
		D3D12_COMMAND_LIST_TYPE GetType() { return m_Type; }
		ID3D12CommandAllocator* GetAllocator() { return m_CurrCommandAllocator.Get(); }

		void TransitionResource(RenderHandle handle, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
		void FlushResourceBarriers();

		// application provides handle, context provides upload memory to move CPU data -> GPU resource
		void CopyBufferRegion(RenderHandle dest, Uint32 destOffset, CPVoid data, Uint32 sizeInBytes);
		void InitTexture(struct Texture& resource, Uint32 numSubresources, const D3D12_SUBRESOURCE_DATA subresources[]);



		void Reset(class CommandQueue* commandQueue);
		Uint64 Submit(bool waitForCompletion);
		GraphicsContext& GetGraphicsContext();
		ComputeContext& GetComputeContext();

		void SetPipelineState(const std::string name);
		void SetPipelineState(const PipelineState& PSO);
		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	
		void PIXBeginEvent(const wchar_t* label);
		void PIXEndEvent();
		void PIXSetMarker(const wchar_t* label);
	protected:
		void BindDescriptorHeaps();
		ResourceAllocation RequestUploadMemory(size_t sizeInBytes, size_t alignment = 256);

		RenderDevice* m_Device = nullptr;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_dCommandList = nullptr;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		D3D12_PRIMITIVE_TOPOLOGY m_Topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		// current memory allocated for recording commands
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CurrCommandAllocator = nullptr;
		
		// Linear Allocator for upload buffers (GPU VISIBLE & CPU WRITABLE) -> usage: copy data from CPU -> GPU 
		UploadResourceAllocator m_CPUAllocator;

		Uint32 m_BarriersToFlush = 0;
		// Resource barrier cache
		D3D12_RESOURCE_BARRIER m_ResourceBarriers[16];

		// Current bound descriptor heaps
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		// SRV/CBV/UAV Shader-Visible Descriptor Heap per command list
		DynamicDescriptorHeap m_ViewDescriptorHeap;
		// Dyanmic Sampler Descriptor Heap Per command List
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
		void SetDynamicCBV(Uint32 rootIndex, Uint32 size, CPVoid data);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& bufferView);
		void SetVertexBuffers(Uint32 offset, Uint32 numBuffers, const D3D12_VERTEX_BUFFER_VIEW& bufferView);

		void DrawInstanced(Uint32 numVerticesPerInstance, Uint32 numInstances,
			Uint32 vertexOffset, Uint32 startInstanceLoc);
		void DrawIndexedInstance(Uint32 instIdxCnt, Uint32 numInstance, Uint32 startIndex, Uint32 vertLoc, Uint32 startInstancePos);

		void Draw(Uint32 numVertices, Uint32 vertexOffset);
		void DrawIndexed(Uint32 numIndices, Uint32 indexOffset, int baseVertexLocation);

		void SetRootSignature(const RootSignature& rootSig);
		void SetDynamicDescriptors(Uint32 rootIndex, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle);
		void SetDynamicSamplers(Uint32 rootIndex, Uint32 offset, Uint32 count, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle);
	private:
	};

	class ComputeContext : public CommandContext {

	public:
		void SetRootSignature(const RootSignature& RootSig);
		void SetConstantArray(Uint32 RootIndex, Uint32 NumConstants, const void* pConstants);
		void SetDescriptorTable(Uint32 rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle);
		void SetDynamicDescriptors(Uint32 rootIndex, Uint32 offset, Uint32 numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE startHandle);

		void Dispatch(Uint32 groupCountX, Uint32 groupCountY, Uint32 groupCountZ);

		//void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
		//void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);
		//void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		//void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);

		//void Dispatch(size_t GroupCountX = 1, size_t GroupCountY = 1, size_t GroupCountZ = 1);
		//void Dispatch1D(size_t ThreadCountX, size_t GroupSizeX = 64);
		//void Dispatch2D(size_t ThreadCountX, size_t ThreadCountY, size_t GroupSizeX = 8, size_t GroupSizeY = 8);
		//void Dispatch3D(size_t ThreadCountX, size_t ThreadCountY, size_t ThreadCountZ, size_t GroupSizeX, size_t GroupSizeY, size_t GroupSizeZ);
		//void DispatchIndirect(GpuBuffer& ArgumentBuffer, uint64_t ArgumentBufferOffset = 0);
		//void ExecuteIndirect(CommandSignature& CommandSig, GpuBuffer& ArgumentBuffer, uint64_t ArgumentStartOffset = 0,
		//	uint32_t MaxCommands = 1, GpuBuffer* CommandCounterBuffer = nullptr, uint64_t CounterOffset = 0);

	//	void SetDynamicDescriptors(Uint32 rootIndex, Uint32 offset, Uint32 count, )
	};
}