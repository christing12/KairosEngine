#pragma once

#include <DirectXTK/SimpleMath.h>

using namespace DirectX::SimpleMath;

namespace Kairos {


	enum class RenderResourceType : Uint8 {
		Buffer = 0x01,
		Texture = 0x02,
		PSO = 0x04,
		RootSignature = 0x08,
		Fence = 0x10,
		Shader = 0x20,
	};
	ENUM_FLAG_OPERATORS(RenderResourceType)


	/* Opaque handle for all types of GPU Resources
		8 bits -> resource type
		rest: magic metadata
	*/
	struct RenderHandle {
		Uint64 handle;
		bool Valid() const { return handle != 0; }
	};

	inline RenderResourceType Magic(const RenderHandle& handle) {
		constexpr uint64_t resourceMask = ((1 << 24) - 1) & ~((1 << 16) - 1);
		return RenderResourceType((handle.handle & resourceMask) >> 16);
	}

	inline RenderResourceType TypeFromHandle(const RenderHandle& handle) {
		constexpr uint64_t indexMask = (1 << 8) - 1;
		return static_cast<RenderResourceType>(handle.handle & indexMask);
	}

	inline Uint64 MetadataFromHandle(const RenderHandle& handle) {
		constexpr Uint64 mask = (1 << 24) - 1;
		const Uint64 metadataMask = ~mask;
		return (handle.handle & metadataMask) >> 24;
	}



	KRS_TYPED_CLASS_ENUM(BindingFlag, Uint16)
	{
		None				= 0u,             ///< The resource will not be bound the pipeline. Use this to create a staging resource
		VertexBuffer		= BIT(0),		///< The resource will be bound as a vertex-mBuffer
		IndexBuffer			= BIT(1),		///< The resource will be bound as a index-mBuffer
		ConstantBuffer		= BIT(2),		///< The resource will be bound as a constant-mBuffer
		StreamOutput		= BIT(3),		///< The resource will be bound to the stream-output stage as an output mBuffer
		ShaderResource		= BIT(4),		///< The resource will be bound as a shader-resource
		UnorderedAccess		= BIT(5),		///< The resource will be bound as an UAV
		RenderTarget		= BIT(6),		 ///< The resource will be bound as a render-target
		DepthStencil		= BIT(7),		///< The resource will be bound as a depth-stencil mBuffer
		IndirectArg			= BIT(8),		 ///< The resource will be bound as an indirect argument mBuffer
	};

	KRS_TYPED_CLASS_ENUM(ShaderType, Uint8)
	{
		Vertex	= 0,
		Pixel	= 1u,
		Compute = 2u,
		Hull,
		Domain,
		Geometry,
	};




	KRS_TYPED_CLASS_ENUM(HeapType, Uint8)
	{
		Default,
		Upload,
		Readback,
	};

	KRS_TYPED_CLASS_ENUM(CommandQueueType, Uint8)
	{

	};

	KRS_TYPED_ENUM(BufferType, Uint8)
	{
		Vertex = 0,
		Index = Vertex + 1,
		Constant = Index + 1,
		NumTypes,
	};

	KRS_TYPED_CLASS_ENUM(RootDescriptorType, Uint8)
	{
		CBV,
		SRV,
		UAV,
	};




	enum BUFFER_TYPE : uint8_t {
		BUFFER_VERTEX = 0,
		BUFFER_INDEX,
		BUFFER_CONSTANT
	};

	enum LINEAR_ALLOCATOR_TYPE : uint8_t {
		GPU_EXCLUSIVE = 1,
		CPU_WRITABLE = GPU_EXCLUSIVE << 1
	};

	enum COMMAND_TYPE : uint8_t {
		COMMAND_TYPE_DIRECT = 0,
		COMMAND_TYPE_COMPUTE = 1,
		COMMAND_TYPE_COPY = 2
	};

	enum class PipelineStateType : uint8_t {
		Graphics = 0,
		Compute
	};
}
