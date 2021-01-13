#pragma once
#include <AssetLoader.h>
namespace assets {

	struct Vertex {
		float position[3];
		float normal[3];
		float tangent[3];
		float bitangent[3];
		float uv[3];
	};

	struct MeshBounds {
		float origin[3];
		float radius;
		float extents[3];
	};

	struct MeshInfo {
		uint64_t vertexBufferSize;
		uint64_t indexBufferSize;
		MeshBounds bounds;



		char indexSize;
		CompressionMode compressionMode;
		std::string originalFile;
	};

	MeshInfo read_mesh_info(AssetFile* file);

	void unpack_mesh(MeshInfo* info, const char* sourceBuffer, size_t sourceSize, char* vertexBuffer, char* indexBuffer);

	AssetFile pack_mesh(MeshInfo* info, char* vertexData, char* indexData);

	MeshBounds calculateBounds(Vertex* vertices, size_t count);
}