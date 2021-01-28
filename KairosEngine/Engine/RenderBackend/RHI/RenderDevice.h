#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>


#include "GraphicsTypes.h"
#include "DescriptorHeap.h"

#include "TextureManager.h"
#include "PipelineStateManager.h"
#include "ShaderManager.h"
#include "ContextManager.h"
#include "BufferManager.h"
#include "CommandQueue.h"

KRS_BEGIN_NAMESPACE(Kairos)

class CommandQueue;
class CommandContext;

class Buffer;
class Texture;
class PipelineState;
class RootSignature;

class TextureManager;
class BufferManager;
class PipelineStateManager;
class ShaderManager;

class RenderDevice {
	friend class SwapChain;
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;
public:
	RenderDevice() = default;
	RenderDevice(Microsoft::WRL::ComPtr<ID3D12Device5> pdDevice);
	~RenderDevice();



	DescriptorRange AllocateDescriptor(DescriptorType type, Uint32 numDescriptors = 1, bool shaderVisible = false);
	CommandContext& AllocateCommandContext(CommandType type);
	CommandQueue& GetCommandQueue(CommandType type);
	void FreeCommandContext(CommandContext& context);
	
	void Flush();
	void Shutdown();

	void TransientCommand(std::function<void(CommandContext&)> func, CommandType type = CommandType::Graphics);

	bool isFenceComplete(Uint64 fenceValue);
	void Present(class SwapChain* swapChain);

	template<class Constants>
	void SetGlobalRootConstant(const Constants& constants);

	template<class FrameConstants>
	void SetFrameRootConstants(const FrameConstants& constants);


	// EXPERIMENTAL
public:
	RenderHandle CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource>, const struct TextureProperties& props);
	Descriptor CreateSampler(const D3D12_SAMPLER_DESC& desc);

	void CompileAll();

// internal functions
public:
	Buffer* CreateStaticBuffer(Uint32 size, Uint32 stride, void* data = nullptr);
	Texture* CreateTexture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, ResourceState state);
	Texture* CreateTexture(const struct TextureProperties& props, ResourceState state);
	Texture* CreateTexture(const char* filename, DXGI_FORMAT format, Uint32 depth = 1, Uint32 mipLevels = 1);
	Buffer* CreateDynConstBuffer(Uint32 sizeinBytes, Uint32 stride, void* data = nullptr);

private:
	Microsoft::WRL::ComPtr<ID3D12Device5> m_dDevice;

	// CPU Descriptor Heaps
	OfflineCBSRVUAVDescriptorAllocator	m_OffCBVSRVUAVAllocator;
	OfflineSamplerDescriptorAllocator	m_OffSamplerAllocator;
	OfflineRTVDescriptorAllocator		m_OffRTVAllocator;
	OfflineDSVDescriptorAllocator		m_OffDSVAllocator;

	OnlineCBVSRVUAVHeap m_OnlineCBV;
	OnlineSamplerHeap m_OnlineSamplerHeap;

	// Command Queues
	Scope<class GraphicsQueue> m_GraphicsQueue;
	Scope<class ComputeQueue> m_ComputeQueue;
	Scope<class CopyQueue> m_CopyQueue;


	//
	Scope<class BufferManager> m_BufferManager;
	Scope<class ShaderManager> m_ShaderManager;
	Scope<class PipelineStateManager> m_PSOManager;
	Scope<class TextureManager> m_TextureManager;

	ContextManager m_ContextManager;

	class DynamicBuffer* m_GlobalBuffer = nullptr;
	class DynamicBuffer* m_PerFrameConstantBuffer = nullptr;
public:
	inline ID3D12Device5* D3DDevice() { return m_dDevice.Get(); }
	inline DynamicBuffer* GlobalBuffer() { return m_GlobalBuffer; }
	inline DynamicBuffer* PerFrameBuffer() { return m_PerFrameConstantBuffer; }
	//class BufferManager* BufferManager();
	//class ShaderManager* ShaderManager();
	//class PipelineStateManager* PipelineStateManager();
	//class TextureManager* TextureManager();

};

template<typename Constant>
void RenderDevice::SetGlobalRootConstant(const Constant& constants)
{
	Uint64 alignedSize = Math::AlignUp(sizeof(Constant), 256);

	if (m_GlobalBuffer == nullptr)

	{

		m_GlobalBuffer = reinterpret_cast<DynamicBuffer*>(m_BufferManager->AllocDynConstBuffer(alignedSize, alignedSize, nullptr).AllocBuffer);
	}

	memcpy(m_GlobalBuffer->DataPtr(), &constants, alignedSize);
}

template<typename FrameConstant>
void RenderDevice::SetFrameRootConstants(const FrameConstant& constants)
{
	Uint64 alignedSize = Math::AlignUp(sizeof(FrameConstant), 256);

	if (m_PerFrameConstantBuffer == nullptr)
	{
		m_PerFrameConstantBuffer = reinterpret_cast<DynamicBuffer*>(m_BufferManager->AllocDynConstBuffer(alignedSize, alignedSize, nullptr).AllocBuffer);
	}
	memcpy(m_PerFrameConstantBuffer->DataPtr(), &constants, alignedSize);
}


KRS_END_NAMESPACE

#include "RenderDevice.inl"