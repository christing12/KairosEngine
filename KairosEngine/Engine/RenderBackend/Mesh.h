#pragma once

#include "GraphicsTypes.h"

namespace Kairos {


	struct Vert {
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
		Vector3 texCoord;
	};



	struct MeshDataComponent
	{
		MeshDataComponent() = default;
		~MeshDataComponent() = default;

		std::vector<Vert> Vertices;
		std::vector<Uint32> Indices;

		static Ref<MeshDataComponent> LoadFromAsset(class RenderDevice* pDevice, const char* filename);
	};
	typedef struct MeshDataComponent MeshDataComponent;

}
