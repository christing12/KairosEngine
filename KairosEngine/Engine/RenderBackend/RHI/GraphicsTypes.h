#pragma once

#include <d3dx12.h>
#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/BitwiseEnum.h>

KRS_BEGIN_NAMESPACE(Kairos)

using GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS;
using CPUHandle = D3D12_CPU_DESCRIPTOR_HANDLE;
using GPUHandle = D3D12_GPU_DESCRIPTOR_HANDLE;

// based off of D3D12_RSEOURCE_STATE
enum class ResourceState : Uint32
{
	Common              = BIT(0),
    ConstantBuffer      = BIT(1),
    UnorderedAccess     = BIT(3),
    DepthWrite          = BIT(4),
    DepthRead           = BIT(5),
	GenericRead         = BIT(6),
	Present             = BIT(7),
	RenderTarget        = BIT(8),
	CopySrc             = BIT(9),
	CopyDest            = BIT(10),
	PixelAccess         = BIT(11),
	NonPixelAccess      = BIT(12),
	IndirectArg         = BIT(13),
	ResolveDest         = BIT(14),
	ResolveSrc          = BIT(15)
};
ENUM_FLAG_OPERATORS(ResourceState);
D3D12_RESOURCE_STATES D3DResourceStates(ResourceState state);


enum class DescriptorType : Uint8 {
	RTV,
	DSV,
	CBV,
	UAV,
	SRV,
	Sampler,
	Count,
};
D3D12_DESCRIPTOR_HEAP_TYPE D3DDescriptorHeap(DescriptorType type);


enum class CommandType : Uint8 {
	Graphics = 0,
	Bundle = 1,
	Compute = 2,
	Copy = 3,
};
D3D12_COMMAND_LIST_TYPE D3DCommandType(CommandType type);


enum class HeapType : Uint8 {
	Default,
	Upload,
	Readback,
	Custom
};
D3D12_HEAP_TYPE D3DHeapType(HeapType type);




enum class BufferType : Uint8
{
	Test
};

enum class ShaderType : Uint8
{
	Vertex = 0,
	Pixel = 1u,
	Compute = 2u,
	Hull,
	Domain,
	Geometry,
};













enum class RenderResourceType : Uint8 {
	Buffer = 0x01,
	Texture = 0x02,
	PSO = 0x04,
	RootSignature = 0x08,
	Fence = 0x10,
	Shader = 0x20,
};
ENUM_FLAG_OPERATORS(RenderResourceType)

struct RenderHandle {
	Uint64 handle = 0;
	bool Valid() const { return handle != 0; }
};

struct QueryResult {
	RenderHandle handle;
};

// Gets Render Resource Type to validate in internal resource manager
inline RenderResourceType TypeFromHandle(const RenderHandle& handle) {
	constexpr uint64_t indexMask = (1 << 8) - 1;
	return static_cast<RenderResourceType>(handle.handle & indexMask);
}


/* --------------------- BUFFER MANAGER INFO -------------------------------- */
inline Uint64 BufferIndexInHandle(const RenderHandle& handle) {
	constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
	return Uint64((handle.handle & resourceMask) >> 8);
}


inline BufferType BufferTypeFromhandle(const RenderHandle& handle)
{
	constexpr Uint64 bufferTypeMask = ((1 << 32) - 1) & ~((1 << 24) - 1);
	return BufferType((handle.handle & bufferTypeMask) >> 24);
}


/* -------------------- END BUFFER MANAGER ---------------------------     */

/* ------------------------ TEXTURE MANAGER --------------------------     */

inline Uint64 TextureIndexFromHandle(const RenderHandle& handle)
{
	constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
	return Uint64((handle.handle & resourceMask) >> 8);
}


inline Uint64 RSIndexFromHandle(const RenderHandle& handle)
{
	constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 8) - 1);
	return Uint64((handle.handle & resourceMask) >> 8);
}

inline Uint64 PSOIndexFromHandle(const RenderHandle& handle)
{
	constexpr uint64_t resourceMask = ((1 << 25) - 1) & ~((1 << 9) - 1);
	return Uint64((handle.handle & resourceMask) >> 9);
}











KRS_END_NAMESPACE



ENABLE_BITMASK_OPERATORS(Kairos::ResourceState);



