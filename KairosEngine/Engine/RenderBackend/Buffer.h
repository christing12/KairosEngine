#pragma once

#include <Core/BaseTypes.h>
#include <Core/EngineCore.h>
#include <RenderBackend/Resource.h>
#include <RenderBackend/DescriptorHeap.h>

KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;



struct Buffer : public GPUResource {
	// Buffer Size = Stride * NumElements
	Uint32 Stride = 0;
	Uint32 NumElements = 0;

	// buffers are suballocated (offset from start of GPU Addres)
	Uint32 Offset = 0; 

	// pointer to CPU writable address if there is one
	void* Data = nullptr; 
};



KRS_END_NAMESPACE