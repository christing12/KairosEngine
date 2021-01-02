#pragma once

#include <Core/EngineCore.h>
#include <Core/BaseTypes.h>


KRS_BEGIN_NAMESPACE(Kairos) 

class Image {
public:
	~Image() = default;

	static Ref<Image> CreateFromFile(const std::string& filename, int channels = 4);

	int Width() const { return mWidth; }
	int Height() const { return mHeight; }
	int Channels() const { return mChannels; }
	bool IsHDR() const {
		return mIsHDR;
	}

	Uint32 BytesPerPixel() const { return mChannels * (mIsHDR ? sizeof(float) : sizeof(unsigned char)); }
	Uint32 Pitch() const { return mWidth * BytesPerPixel(); }

	template<typename T>
	const T* Pixels() const {
		return reinterpret_cast<const T*>(mImageData.get());
	}

private:
	Image();

	int mWidth = 0;
	int mHeight = 0;
	int mChannels = 0;

	bool mIsHDR = false;

	Scope<unsigned char> mImageData;
};


KRS_END_NAMESPACE