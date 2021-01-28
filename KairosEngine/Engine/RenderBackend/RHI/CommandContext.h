#pragma once
#pragma once

#include "UploadResourceAllocator.h"
#include "GraphicsTypes.h"
#include "RootSignature.h"

#define DEFAULT_MAX_BARRIERS 16

namespace Kairos {
	class RenderDevice;
	class GraphicsContext;
	class RootSignature;
	class PipelineState;
	class ComputeContext;
	class GPUResource;
	class Texture;
	class Buffer;

	/* Abstraction of ID3D12CommandList + other context related functions */
	class CommandContext : public std::enable_shared_from_this<CommandContext> {
	public:
		friend class RenderDevice;
		friend class GraphicsContext;
		friend class ComputeContext;
		friend class CommandContext;

		CommandContext(RenderDevice* pDevice, CommandType type);
		~CommandContext();

		//TODO: Fix this:
		void SetScene(class Scene* scene);

		void TransitionResource(GPUResource& resource, ResourceState newState, bool flushImmediate = false);
		void FlushResourceBarriers();

		// application provides handle, context provides upload memory to move CPU data -> GPU resource
		// TODO: Move to CopyContext
		void CopyBufferRegion(RenderHandle dest, Uint32 destOffset, CPVoid data, Uint32 sizeInBytes);
		void CopyBufferRegion(Buffer& buffer, Uint32 destOffset, CPVoid data, Uint32 sizeInBytes);

		void CopyResource(GPUResource& dest, GPUResource& src);
		void CopyTextureRegion(Texture* dest, Texture* src, Uint32 x, Uint32 y, Uint32 z);

		// TODO: Switch Subresource Data for somethign else
		void InitTexture(Texture& texture, Uint32 numSubresources, const D3D12_SUBRESOURCE_DATA subresources[]);

		void Reset(class CommandQueue* commandQueue);
		Uint64 Submit(bool waitForCompletion);
		GraphicsContext& GetGraphicsContext();
		ComputeContext& GetComputeContext();

		void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	protected:
		RenderDevice* m_Device = nullptr;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_dCommandList = nullptr;
		CommandType m_Type = CommandType::Graphics;

		// current memory allocated for recording commands
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CurrCommandAllocator = nullptr;

		// Linear Allocator for upload buffers (GPU VISIBLE & CPU WRITABLE) -> usage: copy data from CPU -> GPU 
		UploadResourceAllocator m_CPUAllocator;


		Uint32 m_BarriersToFlush = 0;
		// Resource barrier cache
		D3D12_RESOURCE_BARRIER m_ResourceBarriers[DEFAULT_MAX_BARRIERS];

		// Current bound descriptor heaps
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		RootSignature* m_CurrRootSig;
		PipelineState* m_CurrPSO;

		class Scene* m_Scene;

	protected:
		void BindDescriptorHeaps();
		DynAlloc RequestUploadMemory(size_t sizeInBytes, size_t alignment = 256);

	public:
		inline ID3D12GraphicsCommandList* D3DCommandList() { return m_dCommandList.Get(); }
		inline const ID3D12GraphicsCommandList* D3DCommandList() const { return m_dCommandList.Get(); }
		inline ID3D12CommandAllocator* GetAllocator() { return m_CurrCommandAllocator.Get(); }
		inline CommandType GetType() { return m_Type; }
		inline Scene* GetScene() { return m_Scene; }
	};

	class GraphicsContext : public CommandContext {
	public:
		~GraphicsContext() = default;
		void ClearColor(class Texture& target);
		void ClearDepth(class Texture& target, D3D12_CLEAR_FLAGS clearFlags, float depth, Uint8 stencil);

		void SetViewportScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect);
		// TODO: Change these to Descriptor from D3D12_CPU_DESCRIPTOR_HANDLE
		void SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[]);
		void SetRenderTargets(Uint32 numRTV, const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[], const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle);

		void SetTopology(D3D_PRIMITIVE_TOPOLOGY);
		void SetShaderConstants(Uint32 rootIndex, Uint32 numConstants, CPVoid data);
		void SetDynamicCBV(Uint32 rootIndex, Uint32 size, CPVoid data);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& bufferView);
		void SetVertexBuffers(Uint32 offset, Uint32 numBuffers, const D3D12_VERTEX_BUFFER_VIEW& bufferView);

		// Draw functions
		void DrawInstanced(Uint32 numVerticesPerInstance, Uint32 numInstances, Uint32 vertexOffset, Uint32 startInstanceLoc);
		void DrawIndexedInstance(Uint32 instIdxCnt, Uint32 numInstance, Uint32 startIndex, Uint32 vertLoc, Uint32 startInstancePos);
		void Draw(Uint32 numVertices, Uint32 vertexOffset);
		void DrawIndexed(Uint32 numIndices, Uint32 indexOffset, int baseVertexLocation);


		void SetConstants(Uint32 rootIdx, Uint32 numConstants, CPVoid pConstants);
		void SetRootSignature(RootSignature* rootSig);
		void SetPipelineState(const std::string name);
		void BindCommonResources(class RootSignature* rootSig);
		void BindPassConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS address);
		void BindBuffer(class Buffer& buffer, Uint16 shaderRegister, Uint16 registerSpace, ShaderRegister registerType);
	};





	class ComputeContext : public CommandContext {
	public:
		~ComputeContext() = default;
		void SetRootSignature(RootSignature* RootSig);
		void SetConstantArray(Uint32 RootIndex, Uint32 NumConstants, const void* pConstants);
		void SetDescriptorTable(Uint32 rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE startGPUHandle);
		void SetRootConstants(Uint32 rootIdx, Uint32 space, Uint32 numConstants, CPVoid data);
		void Dispatch(Uint32 groupCountX, Uint32 groupCountY, Uint32 groupCountZ);
		void BindPassConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS address);


		void SetPipelineState(const std::string name);
	private:
		void BindCommonResources(class RootSignature* rootSig);
	};
}