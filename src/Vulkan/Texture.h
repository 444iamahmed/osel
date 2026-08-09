#pragma once

#include <string>

#include "VulkanDevice.hpp"

namespace OselEngine::Vulkan {
typedef unsigned char* ImageData;

class Texture {
    public:
    Texture(std::string);
    ~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	void loadImage(const std::string& path);

    private:
	ImageData mImageData;

	int32_t mWidth{0}, mHeight{0}, mChannels{0};
	int32_t mBytesPerPixel{0};

	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkImageView mImageView;
	VkSampler mSampler;
};
}
