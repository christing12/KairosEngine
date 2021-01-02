#pragma once

#include <vector>
#include <string>

// stateless header
namespace assets {
	// stores info about file -> do not want to store this data anywhere
	struct AssetFile {
		char type[4]; // identifier for whether its texture mesh, etc...
		int version; // major & minor version for breaking changes
		std::string json; // metadata about asset
		std::vector<char> binaryBlob; // actual data
	};

	enum class CompressionMode : uint32_t {
		None,
		LZ4
	};

	bool save_binaryfile(const char* path, const AssetFile& file);

	bool load_binaryfile(const char* path, AssetFile& outfile);

	assets::CompressionMode parse_compression(const char* f);
}
