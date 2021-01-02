#include "AssetLoader.h"

#include <ostream>
#include <fstream>
#include <iostream>

namespace assets {
    bool assets::save_binaryfile(const char* path, const AssetFile& file)
    {
        std::ofstream outfile;
        outfile.open(path, std::ios::binary | std::ios::out); // binary outfile

        outfile.write(file.type, 4); // writes type in

        uint32_t version = file.version;
        outfile.write((const char*)&version, sizeof(uint32_t));

        uint32_t metadataLen = file.json.size();
        outfile.write((const char*)&metadataLen, sizeof(uint32_t));

        uint32_t blobLen = file.binaryBlob.size();
        outfile.write((const char*)&blobLen, sizeof(uint32_t));

        outfile.write(file.json.data(), metadataLen);

        outfile.write(file.binaryBlob.data(), blobLen);
        outfile.close();

        return true;
    }


    bool assets::load_binaryfile(const char* path, AssetFile& outfile)
    {
        std::ifstream infile;
        infile.open(path, std::ios::binary);

        if (!infile.is_open()) return false;

        infile.seekg(0);

        infile.read(outfile.type, 4);

        uint32_t vers;
        infile.read((char*)&outfile.version, sizeof(uint32_t));

        uint32_t metadataLen;
        infile.read((char*)&metadataLen, sizeof(uint32_t));

        uint32_t blobLen;
        infile.read((char*)&blobLen, sizeof(uint32_t));

        outfile.json.resize(metadataLen);
        infile.read(outfile.json.data(), metadataLen);

        outfile.binaryBlob.resize(blobLen);
        infile.read(outfile.binaryBlob.data(), blobLen);

        return true;
    }

    assets::CompressionMode parse_compression(const char* f)
    {
        if (strcmp(f, "LZ4") == 0)
            return assets::CompressionMode::LZ4;
        else
            return assets::CompressionMode::None;
    }

}
