#include "MeshAsset.h"

#include "json.hpp"
#include "lz4.h"

using namespace assets;


MeshInfo assets::read_mesh_info(AssetFile* file) {
	using nlohmann::json;

	MeshInfo info;
	json metadata = json::parse(file->json);

	info.vertexBufferSize = metadata["vertex_buffer_size"];
	info.indexBufferSize = metadata["index_buffer_size"];
	info.indexSize = (uint8_t)metadata["index_size"];
	info.originalFile = metadata["original_file"];

	std::string compressionString = metadata["compression"];
	info.compressionMode = parse_compression(compressionString.c_str());


	{
		std::vector<float> boundsData;
		boundsData.reserve(7);
		boundsData = metadata["bounds"].get<std::vector<float>>();

		info.bounds.origin[0] = boundsData[0];
		info.bounds.origin[1] = boundsData[1];
		info.bounds.origin[2] = boundsData[2];

		info.bounds.radius = boundsData[3];

		info.bounds.extents[0] = boundsData[4];
		info.bounds.extents[1] = boundsData[5];
		info.bounds.extents[2] = boundsData[6];
	}


	return info;
}

void assets::unpack_mesh(MeshInfo* info, const char* sourceBuffer, size_t sourceSize, char* vertexBuffer, char* indexBuffer) {
	std::vector<char> decompressedBuffer;
	decompressedBuffer.resize(info->vertexBufferSize + info->indexBufferSize);
	LZ4_decompress_safe(sourceBuffer, decompressedBuffer.data(), static_cast<int>(sourceSize), static_cast<int>(decompressedBuffer.size()));

	memcpy(vertexBuffer, decompressedBuffer.data(), info->vertexBufferSize);

	memcpy(indexBuffer, decompressedBuffer.data() + info->vertexBufferSize, info->indexBufferSize);
}

AssetFile assets::pack_mesh(MeshInfo* info, char* vertexData, char* indexData) {
	AssetFile file;
	file.type[0] = 'M';
	file.type[1] = 'E';
	file.type[2] = 'S';
	file.type[3] = 'H';
	file.version = 1;

	using nlohmann::json;

	json metadata;
	metadata["vertex_buffer_size"] = info->vertexBufferSize;
	metadata["index_buffer_size"] = info->indexBufferSize;
	metadata["index_size"] = info->indexSize;
	metadata["original_file"] = info->originalFile;

	
	std::vector<float> boundsData;
	boundsData.resize(7);

	boundsData[0] = info->bounds.origin[0];
	boundsData[1] = info->bounds.origin[1];
	boundsData[2] = info->bounds.origin[2];

	boundsData[3] = info->bounds.radius;

	boundsData[4] = info->bounds.extents[0];
	boundsData[5] = info->bounds.extents[1];
	boundsData[6] = info->bounds.extents[2];

	metadata["bounds"] = boundsData;

	size_t fullsize = info->vertexBufferSize + info->indexBufferSize;

	std::vector<char> merged_buffer;
	merged_buffer.resize(fullsize);

	//copy vertex buffer
	memcpy(merged_buffer.data(), vertexData, info->vertexBufferSize);

	//copy index buffer
	memcpy(merged_buffer.data() + info->vertexBufferSize, indexData, info->indexBufferSize);
	

	size_t compressStaging = LZ4_compressBound(static_cast<int>(fullsize));
	file.binaryBlob.resize(compressStaging);

	int compressedSize = LZ4_compress_default(merged_buffer.data(), file.binaryBlob.data(), static_cast<int>(merged_buffer.size()), static_cast<int>(compressStaging));

	file.binaryBlob.resize(compressedSize);

	metadata["compression"] = "LZ4";
	file.json = metadata.dump();

	return file;

	
}

MeshBounds assets::calculateBounds(Vertex* vertices, size_t count) {
	MeshBounds bounds;

	float min[3] = { std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max() };
	float max[3] = { std::numeric_limits<float>::min(),std::numeric_limits<float>::min(),std::numeric_limits<float>::min() };

	for (int i = 0; i < count; i++) {
		min[0] = std::min(min[0], vertices[i].position[0]);
		min[1] = std::min(min[1], vertices[i].position[1]);
		min[2] = std::min(min[2], vertices[i].position[2]);

		max[0] = std::max(max[0], vertices[i].position[0]);
		max[1] = std::max(max[1], vertices[i].position[1]);
		max[2] = std::max(max[2], vertices[i].position[2]);
	}

	bounds.extents[0] = (max[0] - min[0]) / 2.0f;
	bounds.extents[1] = (max[1] - min[1]) / 2.0f;
	bounds.extents[2] = (max[2] - min[2]) / 2.0f;

	bounds.origin[0] = bounds.extents[0] + min[0];
	bounds.origin[1] = bounds.extents[1] + min[1];
	bounds.origin[2] = bounds.extents[2] + min[2];

	//go through the vertices again to calculate the exact bounding sphere radius
	float r2 = 0;
	for (int i = 0; i < count; i++) {

		float offset[3];
		offset[0] = vertices[i].position[0] - bounds.origin[0];
		offset[1] = vertices[i].position[1] - bounds.origin[1];
		offset[2] = vertices[i].position[2] - bounds.origin[2];

		//pithagoras
		float distance = offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2];
		r2 = std::max(r2, distance);
	}

	bounds.radius = std::sqrt(r2);

	return bounds;
}