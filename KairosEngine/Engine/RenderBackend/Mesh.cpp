#include "krspch.h"
#include "Mesh.h"
#include "RenderDevice.h"
#include "Buffer.h"

#include "GraphicsTypes.h"
#include "CommandContext.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <MeshAsset.h>

namespace Kairos {

	Ref<MeshDataComponent> MeshDataComponent::LoadFromAsset(RenderDevice* pDevice, const char* filename)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(filename, file);
		if (!loaded) {
			KRS_CORE_ERROR("error when loading mesh");
			return false;
		}

		assets::MeshInfo meshinfo = assets::read_mesh_info(&file);
		std::vector<char> vertexBuffer;
		std::vector<char> indexBuffer;

		vertexBuffer.resize(meshinfo.vertexBufferSize);
		indexBuffer.resize(meshinfo.indexBufferSize);

		assets::unpack_mesh(&meshinfo, file.binaryBlob.data(), file.binaryBlob.size(), vertexBuffer.data(), indexBuffer.data());

		Ref<MeshDataComponent> mesh = CreateRef<MeshDataComponent>();
		mesh->Indices.clear();
		mesh->Vertices.clear();

		mesh->Indices.resize(indexBuffer.size() / sizeof(uint32_t));
		for (int i = 0; i < mesh->Indices.size(); i++) {
			uint32_t* unpacked_indices = (uint32_t*)indexBuffer.data();
			mesh->Indices[i] = unpacked_indices[i];
		}

		assets::Vertex* unpackedVertices = (assets::Vertex*)vertexBuffer.data();
		mesh->Vertices.resize(vertexBuffer.size() / sizeof(assets::Vertex));

		for (int i = 0; i < mesh->Vertices.size(); i++) {
			mesh->Vertices[i].position.x = unpackedVertices[i].position[0];
			mesh->Vertices[i].position.y = unpackedVertices[i].position[1];
			mesh->Vertices[i].position.z = unpackedVertices[i].position[2];

			mesh->Vertices[i].normal.x = unpackedVertices[i].normal[0];
			mesh->Vertices[i].normal.y = unpackedVertices[i].normal[1];
			mesh->Vertices[i].normal.z = unpackedVertices[i].normal[2];


			mesh->Vertices[i].tangent.x = unpackedVertices[i].tangent[0];
			mesh->Vertices[i].tangent.y = unpackedVertices[i].tangent[1];
			mesh->Vertices[i].tangent.z = unpackedVertices[i].tangent[2];

			mesh->Vertices[i].bitangent.x = unpackedVertices[i].bitangent[0];
			mesh->Vertices[i].bitangent.y = unpackedVertices[i].bitangent[1];
			mesh->Vertices[i].bitangent.z = unpackedVertices[i].bitangent[2];

			mesh->Vertices[i].texCoord.x = unpackedVertices[i].uv[0];
			mesh->Vertices[i].texCoord.y = unpackedVertices[i].uv[1];

		}
		return mesh;
	}
}