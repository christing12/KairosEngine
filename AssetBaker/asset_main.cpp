#include <iostream>
#include <json.hpp>
#include <fstream>
#include <filesystem>

#include <chrono>
#include <AssetLoader.h>
#include <TextureAsset.h>
#include <MeshAsset.h>
#include <DirectXTK/SimpleMath.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


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
std::string calculate_assimp_mesh_name(const aiScene* scene, int meshIndex)
{
	char buffer[50];

	itoa(meshIndex, buffer, 10);
	std::string matname = "MESH_" + std::string{ buffer } + "_" + std::string{ scene->mMeshes[meshIndex]->mName.C_Str() };
	return matname;
}


void extract_assimp_meshes(const aiScene* scene, const fs::path& input, const fs::path& outputFolder, const ConverterState& convState) {
	for (int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
		auto mesh = scene->mMeshes[meshIndex];

		using VertexFormat = assets::Vertex;

		std::string meshName = calculate_assimp_mesh_name(scene, meshIndex);
		
		std::vector<VertexFormat> vertices;
		std::vector<uint32_t> indices;

		vertices.resize(mesh->mNumVertices);
		for (int v = 0; v < mesh->mNumVertices; v++) {
			VertexFormat vert;
			vert.position[0] = mesh->mVertices[v].x;
			vert.position[1] = mesh->mVertices[v].y;
			vert.position[2] = mesh->mVertices[v].z;

			if (mesh->HasNormals()) {
				vert.normal[0] = mesh->mNormals[v].x;
				vert.normal[1] = mesh->mNormals[v].y;
				vert.normal[2] = mesh->mNormals[v].z;
			}

			if (mesh->HasTangentsAndBitangents()) {
				vert.tangent[0] = mesh->mTangents[v].x;
				vert.tangent[1] = mesh->mTangents[v].y;
				vert.tangent[2] = mesh->mTangents[v].z;

				vert.bitangent[0] = mesh->mBitangents[v].x;
				vert.bitangent[1] = mesh->mBitangents[v].y;
				vert.bitangent[2] = mesh->mBitangents[v].z;
			}

			if (mesh->GetNumUVChannels() >= 1) {
				vert.uv[0] = mesh->mTextureCoords[0][v].x;
				vert.uv[1] = mesh->mTextureCoords[0][v].y;
			}

			vertices[v] = vert;
		}

		indices.resize(mesh->mNumFaces * 3);
		for (int f = 0; f < mesh->mNumFaces; f++) {
			indices[f * 3 + 0] = mesh->mFaces[f].mIndices[0];
			indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
			indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];

			if (input.extension() == ".fbx") {

			}

		}

		MeshInfo meshInfo;
		meshInfo.vertexBufferSize = vertices.size() * sizeof(Vertex);
		meshInfo.indexBufferSize = indices.size() * sizeof(uint32_t);
		meshInfo.indexSize = sizeof(uint32_t);

		meshInfo.originalFile = input.string();
		meshInfo.bounds = assets::calculateBounds(vertices.data(), vertices.size());

		assets::AssetFile newFile = assets::pack_mesh(&meshInfo, (char*)vertices.data(), (char*)indices.data());

		fs::path meshpath = outputFolder / (meshName + ".mesh");
		save_binaryfile(meshpath.string().c_str(), newFile);
	
	}
}



using namespace DirectX::SimpleMath;
int main(int argc, char* argv[]) {
	Vector3 test = Vector3::Zero;
	std::cout << test.x << " " << test.y << " " << test.z << std::endl;
	if (argc < 3) {
		std::cout << "Command Line needs path to info file\n";
		return -1;
	}
	else {
		//{
		//	fs::path path{ argv[1] };
		//	fs::path directory = path;
		//	fs::path exported_dir = path.parent_path() / "assets_export";

		//	std::cout << "Loaded asset directory at " << directory << std::endl;


		//	ConverterState convState{ path, exported_dir };


		//	for (auto& p : fs::recursive_directory_iterator(directory)) {
		//		std::cout << "File: " << p;
		//		auto relative = p.path().lexically_proximate(directory);

		//		auto export_path = exported_dir / relative;
		//		if (!fs::is_directory(export_path.parent_path()))
		//			fs::create_directory(export_path.parent_path());

		//		if (p.path().extension() == ".png" || p.path().extension() == ".jpg" || p.path().extension() == ".TGA"
		//			|| p.path().extension() == ".hdr")
		//		{
		//			std::cout << "found a texture" << std::endl;

		//			auto newpath = p.path();
		//			export_path.replace_extension(".tx");
		//			convert_image(p.path(), export_path);
		//		}
		//	}
		//}

		{
			fs::path path{ argv[2] };
			fs::path directory = path;
			fs::path exported_dir = path.parent_path() / "assets_export";

			std::cout << "Loaded asset directory at " << directory << std::endl;


			ConverterState convState{ path, exported_dir };


			for (auto& p : fs::recursive_directory_iterator(directory)) {
				std::cout << "File: " << p;
				auto relative = p.path().lexically_proximate(directory);

				auto export_path = exported_dir / relative;
				if (!fs::is_directory(export_path.parent_path()))
					fs::create_directory(export_path.parent_path());

				if (p.path().extension() == ".fbx" || p.path().extension() == ".obj") {
					Assimp::Importer importer;

					auto start1 = std::chrono::system_clock::now();
					const aiScene* scene = importer.ReadFile(p.path().string(),
						aiProcess_OptimizeMeshes	|
						aiProcess_GenNormals		|
						aiProcess_CalcTangentSpace	|
						aiProcess_FlipUVs			|
						aiProcess_MakeLeftHanded	|
						aiProcess_Triangulate		|
						aiProcess_JoinIdenticalVertices);

					auto folder = export_path.parent_path() / (p.path().stem().string() + "_Mesh");
					fs::create_directory(folder);

					extract_assimp_meshes(scene, p.path(), folder, convState);
				}
			}
		}
		

	}




	return 0;
}


fs::path ConverterState::convert_to_export_relative(fs::path path) const {
	return path.lexically_proximate(export_path);
}