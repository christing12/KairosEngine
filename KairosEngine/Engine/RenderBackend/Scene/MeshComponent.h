#pragma once


#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>

KRS_BEGIN_NAMESPACE(Kairos)

class Mesh;
class Material;


struct MeshComponent {
	Mesh* Mesh;
	Material* Mat;

	Uint32 gpuIndex;
};



KRS_END_NAMESPACE