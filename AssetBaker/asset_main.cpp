#include <iostream>
#include <json.hpp>
#include <fstream>
#include <filesystem>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <AssetLoader.h>
#include <TextureAsset.h>
#include <assimp/Importer.hpp>

#include <DirectXTK/SimpleMath.h>

namespace fs = std::filesystem;

using namespace assets;

struct ConverterState {
	fs::path asset_path;
	fs::path export_path;

	fs::path convert_to_export_relative(fs::path path) const;
};

bool convert_image(const fs::path& input, const fs::path& output) {
	int texWidth, texHeight, texChannels;
	std::string fullPath = input.string();
	std::string extension = input.extension().string();
	
	std::string thing = fullPath.erase(fullPath.find(extension), extension.length());
	std::string end = thing.substr(thing.length() - 2, 2);
	
	int optionalForceChannels = 0;
	TextureInfo texinfo;

	if (end == "_A" || end == "_N")
		optionalForceChannels = 4;

	unsigned char* pixels;
	if (stbi_is_hdr(input.u8string().c_str())) {
		texinfo.isHDR = true;
		optionalForceChannels = 4;
		float* data = stbi_loadf(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, optionalForceChannels);
		if (data != nullptr)
			pixels = reinterpret_cast<unsigned char*>(data);
	}
	else {
		texinfo.isHDR = false;
		stbi_uc* data = stbi_load(input.u8string().c_str(), &texWidth, &texHeight, &texChannels, optionalForceChannels);
		pixels = data;
	}

	texChannels = optionalForceChannels == 0 ? texChannels : optionalForceChannels;
	if (!pixels) {
		std::cout << "Failed to load texture file " << input << std::endl;
		return false;
	}

	int texture_size = texWidth * texHeight * (texChannels * (texinfo.isHDR ? sizeof(float) : sizeof(unsigned char)));

	texinfo.textureSize = texture_size;
	texinfo.pixelSize[0] = texWidth;
	texinfo.pixelSize[1] = texHeight;
	texinfo.pixelSize[2] = texChannels;
	texinfo.textureFormat = TextureFormat::RGBA8;
	texinfo.originalFile = input.string();
	assets::AssetFile newImage = assets::pack_texture(&texinfo, pixels);

	stbi_image_free(pixels);

	save_binaryfile(output.string().c_str(), newImage);

	return true;
}


using namespace DirectX::SimpleMath;
int main(int argc, char* argv[]) {
	Vector3 test = Vector3::Zero;
	std::cout << test.x << " " << test.y << " " << test.z << std::endl;
	if (argc < 2) {
		std::cout << "Command Line needs path to info file\n";
		return -1;
	}
	else {
		fs::path path{ argv[1] };
		fs::path directory = path;
		fs::path exported_dir = path.parent_path() / "assets_export";

		std::cout << "Loaded asset directory at " << directory << std::endl;

		for (auto& p : fs::recursive_directory_iterator(directory)) {
			std::cout << "File: " << p;
			auto relative = p.path().lexically_proximate(directory);

			auto export_path = exported_dir / relative;
			if (!fs::is_directory(export_path.parent_path()))
				fs::create_directory(export_path.parent_path());

			if (p.path().extension() == ".png" || p.path().extension() == ".jpg" || p.path().extension() == ".TGA"
				|| p.path().extension() == ".hdr")
			{
				std::cout << "found a texture" << std::endl;

				auto newpath = p.path();
				export_path.replace_extension(".tx");
				convert_image(p.path(), export_path);
			}
		}

	}
	return 0;
}