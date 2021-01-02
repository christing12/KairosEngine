#include "TextureAsset.h"

#include <iostream>
#include <json.hpp>
#include <lz4.h>


assets::TextureFormat parse_format(const char* f) {
	if (strcmp(f, "RGBA8") == 0)
		return assets::TextureFormat::RGBA8;
	else
		return assets::TextureFormat::Unknown;
}

namespace assets {
	TextureInfo read_texture_info(AssetFile* file) {
		using nlohmann::json;

		TextureInfo info;

		json texture_metadata = json::parse(file->json);
		std::string format = texture_metadata["format"];
		info.textureFormat = parse_format(format.c_str());

		std::string compressionString = texture_metadata["compression"];
		info.compressionMode = parse_compression(compressionString.c_str());

		info.pixelSize[0] = texture_metadata["width"];
		info.pixelSize[1] = texture_metadata["height"];
		info.pixelSize[2] = texture_metadata["channels"];
		info.textureSize = texture_metadata["buffer_size"];
		info.originalFile = texture_metadata["original_file"];
		info.isHDR = texture_metadata["isHDR"];

		//for (auto& [key, value] : texture_metadata["pages"].items()) {
		//	PageInfo page;

		//	page.compressedSize = value["compressed_size"];
		//	page.originalSize = value["original_size"];
		//	page.width = value["width"];
		//	page.height = value["value"];

		//	info.pages.push_back(page);
		//}

		return info;
	}

	void unpack_texture(TextureInfo* info, const char* sourceBuffer, size_t sourceSize, char* destination) {
		if (info->compressionMode == CompressionMode::LZ4) {
			LZ4_decompress_safe(sourceBuffer, destination, sourceSize, info->textureSize);

		}
		else {
			memcpy(destination, sourceBuffer, sourceSize);
		}
	}

	void unpack_texture_page(TextureInfo* info, int pageIndex, char* sourceBuffer, char* destination) {
		//char* source = sourceBuffer;
		//for (int i = 0; i < pageIndex; i++) {
		//	source += info->pages[i].compressedSize;
		//}

		//if (info->compressionMode == CompressionMode::LZ4) {



		//	//size doesnt fully match, its compressed
		//	if (info->pages[pageIndex].compressedSize != info->pages[pageIndex].originalSize)
		//	{
		//		LZ4_decompress_safe(source, destination, info->pages[pageIndex].compressedSize, info->pages[pageIndex].originalSize);
		//	}
		//	else {
		//		//size matched, uncompressed page
		//		memcpy(destination, source, info->pages[pageIndex].originalSize);
		//	}
		//}
		//else {
		//	memcpy(destination, source, info->pages[pageIndex].originalSize);
		//}
	}

	AssetFile pack_texture(TextureInfo* info, void* pixelData) {
		using nlohmann::json;
		json texture_metadata;
		texture_metadata["format"] = "RGBA8";
		texture_metadata["width"] = info->pixelSize[0];
		texture_metadata["height"] = info->pixelSize[1];
		texture_metadata["channels"] = info->pixelSize[2];
		texture_metadata["buffer_size"] = info->textureSize;
		texture_metadata["original_file"] = info->originalFile;
		texture_metadata["isHDR"] = info->isHDR;


		//core file header
		AssetFile file;
		file.type[0] = 'T';
		file.type[1] = 'E';
		file.type[2] = 'X';
		file.type[3] = 'I';
		file.version = 1;

		int compressStaging = LZ4_compressBound(info->textureSize);

		file.binaryBlob.resize(compressStaging);
		int compressedSize = LZ4_compress_default((const char*)pixelData, file.binaryBlob.data(), info->textureSize, compressStaging);

		file.binaryBlob.resize(compressedSize);
		texture_metadata["compression"] = "LZ4";

		std::string stringified = texture_metadata.dump();
		file.json = stringified;

		return file;
	}
}