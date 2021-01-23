#include "krspch.h"
#include "Mesh.h"

#include "RenderBackend/RHI/GraphicsTypes.h"

#include <MeshAsset.h>

namespace Kairos {
	Mesh Mesh::LoadFromAsset(RenderDevice* pDevice, const std::string& filename)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryfile(filename.c_str(), file);
		if (!loaded) {
			KRS_CORE_ERROR("error when loading mesh");
			return Mesh{};
		}

		assets::MeshInfo meshinfo = assets::read_mesh_info(&file);
		std::vector<char> vertexBuffer;
		std::vector<char> indexBuffer;

		vertexBuffer.resize(meshinfo.vertexBufferSize);
		indexBuffer.resize(meshinfo.indexBufferSize);

		assets::unpack_mesh(&meshinfo, file.binaryBlob.data(), file.binaryBlob.size(), vertexBuffer.data(), indexBuffer.data());

		Mesh mesh;
		mesh.m_Vertices.clear();
		mesh.m_Indices.clear();

		mesh.m_Indices.resize(indexBuffer.size() / sizeof(uint32_t));
		for (int i = 0; i < mesh.m_Indices.size(); i++) {
			uint32_t* unpacked_indices = (uint32_t*)indexBuffer.data();
			mesh.m_Indices[i] = unpacked_indices[i];
		}

		assets::Vertex* unpackedVertices = (assets::Vertex*)vertexBuffer.data();
		mesh.m_Vertices.resize(vertexBuffer.size() / sizeof(assets::Vertex));

		for (int i = 0; i < mesh.m_Vertices.size(); i++) {
			mesh.m_Vertices[i].position.x = unpackedVertices[i].position[0];
			mesh.m_Vertices[i].position.y = unpackedVertices[i].position[1];
			mesh.m_Vertices[i].position.z = unpackedVertices[i].position[2];

			mesh.m_Vertices[i].normal.x = unpackedVertices[i].normal[0];
			mesh.m_Vertices[i].normal.y = unpackedVertices[i].normal[1];
			mesh.m_Vertices[i].normal.z = unpackedVertices[i].normal[2];


			mesh.m_Vertices[i].tangent.x = unpackedVertices[i].tangent[0];
			mesh.m_Vertices[i].tangent.y = unpackedVertices[i].tangent[1];
			mesh.m_Vertices[i].tangent.z = unpackedVertices[i].tangent[2];

			mesh.m_Vertices[i].bitangent.x = unpackedVertices[i].bitangent[0];
			mesh.m_Vertices[i].bitangent.y = unpackedVertices[i].bitangent[1];
			mesh.m_Vertices[i].bitangent.z = unpackedVertices[i].bitangent[2];

			mesh.m_Vertices[i].texCoord.x = unpackedVertices[i].uv[0];
			mesh.m_Vertices[i].texCoord.y = unpackedVertices[i].uv[1];
		}
		return mesh;
	}

}

