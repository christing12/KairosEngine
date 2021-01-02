#pragma once
#include "AssetLoader.h"

namespace assets {
	enum class TextureFormat : uint32_t {
		Unknown = 0,
		RGBA8,
		R8
	};

	struct PageInfo {
		uint32_t width;
		uint32_t height;
		uint32_t compressedSize;
		uint32_t originalSize;
	};

	struct TextureInfo {
		uint64_t textureSize;
		TextureFormat textureFormat;
		CompressionMode compressionMode;

		bool isHDR = false;
		uint32_t pixelSize[3];

		std::string originalFile;
		//std::vector<PageInfo> pages;
	};
	// parses json metadata

	TextureInfo read_texture_info(AssetFile* file); 

	// moves binary blob and decompresses to dest buffer
	void unpack_texture(TextureInfo* info, const char* sourceBuffer, size_t sourceSize, char* destination); 

	void unpack_texture_page(TextureInfo* info, int pageIndex, char* sourceBuffer, char* destination);

	AssetFile pack_texture(TextureInfo* info, void* pixelData);
}