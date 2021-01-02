#include "krspch.h"
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

namespace Kairos {
	Image::Image()
		: mWidth(0)
		, mHeight(0)
		, mChannels(0)
		, mIsHDR(false)
	{}


	Ref<Image> Image::CreateFromFile(const std::string& filename, int channels)
	{
		std::shared_ptr<Image> img{ new Image };
		if (stbi_is_hdr(filename.c_str())) {
			float* imgData = stbi_loadf(filename.c_str(), &img->mWidth, &img->mHeight, &img->mChannels, channels);
			if (imgData != nullptr) {
				img->mImageData.reset(reinterpret_cast<unsigned char*>(imgData));
				img->mIsHDR = true;
			}
		}
		else {
			unsigned char* imgData = stbi_load(filename.c_str(), &img->mWidth, &img->mHeight, &img->mChannels, channels);
			if (imgData != nullptr) {
				img->mImageData.reset(imgData);
				img->mIsHDR = false;
			}
		}

		img->mChannels = channels > 0 ? channels : img->mChannels;
		return img;
	}

}

