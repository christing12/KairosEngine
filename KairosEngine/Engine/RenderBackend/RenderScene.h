#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>
#include <Core/ClassTemplates.h>

#include "Buffer.h"


KRS_BEGIN_NAMESPACE(Kairos)

class RenderDevice;
class BufferManager;


struct DrawableMeshComponent
{
	Uint32 NumVertices = 0;
	Uint32 NumIndices = 0;
	Uint32 FirstVertex = 0;
	Uint32 FirstIndex = 0;
	bool isMerged = false;

	struct MeshDataComponent* original;
};

class RenderScene {
public:
	KRS_CLASS_NON_COPYABLE_AND_MOVABLE(RenderScene);
	
	void Init(RenderDevice* pDevice);

	void MergeMeshes(BufferManager* manager);
private:
	RenderDevice* m_Device;

	RenderHandle m_MergedVertices;
	RenderHandle m_MergedIndices;
	std::vector<DrawableMeshComponent> m_Meshes;

};

KRS_END_NAMESPACE