#include "Texture.h"

#include "stdafx.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace OselEngine {
namespace Vulkan {
Texture::Texture(std::string path, Device& device) {
    loadImage(path);

    VkDeviceSize size = mWidth * mHeight * mChannels * sizeof(int);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    VkCommandBuffer commandBuffer;

    device.createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory);


    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(device.get(), &allocInfo, &commandBuffer);
    device.beginSingleTimeCommands(commandBuffer);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = mWidth;
    region.imageExtent.height = mHeight;
    region.imageExtent.depth = 1;

    //vkCmdPipelineBarrier(commandBuffer, )
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    device.endSingleTimeCommands(commandBuffer);

    vkDeviceWaitIdle(device.get());
}

Texture::~Texture() {
}

void Texture::loadImage(const std::string& path) {
    spdlog::info("loading image");

    mImageData = stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, STBI_default);
}

} // Vulkan
} // PhoenixEngine