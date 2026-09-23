#include "Texture.hpp"

#include "Vulkan/VulkanDevice.hpp"
#include "stdafx.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace OselEngine::Vulkan {
Texture::Texture(std::string path, Device& device) : mDevice{device} {
	loadImage(path);

	VkDeviceSize size = mWidth * mHeight * STBI_rgb_alpha;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkCommandBuffer commandBuffer;

	mDevice.createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);

	void* data;
	vkMapMemory(mDevice.get(), stagingMemory, 0, size, 0, &data);
	memcpy(data, mImageData, size);
	vkUnmapMemory(mDevice.get(), stagingMemory);

	mDevice.createImage(
		mWidth, mHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory
	);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mDevice.getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	vkAllocateCommandBuffers(mDevice.get(), &allocInfo, &commandBuffer);
	mDevice.beginSingleTimeCommands(commandBuffer);

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

	VkImageMemoryBarrier preImageMemoryBarrier{};
	preImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	preImageMemoryBarrier.image = mImage;
	preImageMemoryBarrier.srcAccessMask = 0;
	preImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	preImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	preImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	preImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	preImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;

	preImageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &preImageMemoryBarrier);

	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	VkImageMemoryBarrier postImageMemoryBarrier{};
	postImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	postImageMemoryBarrier.image = mImage;
	postImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	postImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	postImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	postImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	postImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	postImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	postImageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &postImageMemoryBarrier);

	mDevice.endSingleTimeCommands(commandBuffer);

	vkFreeMemory(mDevice.get(), stagingMemory, nullptr);
	vkDestroyBuffer(mDevice.get(), stagingBuffer, nullptr);

	stbi_image_free(mImageData);

	mSampler = mDevice.getSampler(SamplerType::BASIC);

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.image = mImage;
	imageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageViewCreateInfo.subresourceRange = subresourceRange;

	if (vkCreateImageView(mDevice.get(), &imageViewCreateInfo, nullptr, &mImageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view");
	}
}

Texture::~Texture() {
	vkDestroyImageView(mDevice.get(), mImageView, nullptr);
	vkDestroyImage(mDevice.get(), mImage, nullptr);
	vkFreeMemory(mDevice.get(), mImageMemory, nullptr);
}

void Texture::loadImage(const std::string& path) {
	spdlog::info("loading image");

	mImageData =
		stbi_load(path.c_str(), &mWidth, &mHeight, &mChannels, STBI_rgb_alpha);

	spdlog::info("got an image?: {}", mChannels);

	if (mImageData == nullptr) {
		spdlog::info("got an image?");
		// This will print the precise reason (e.g., "can't fopen", "bad png sig", etc.)
		const char* error_msg = stbi_failure_reason();
		std::cout << "STB Error: " << (error_msg ? error_msg : "Unknown error") << std::endl;
	}
	spdlog::info("image width {}, image height {}", mWidth, mHeight);
}

} // namespace OselEngine::Vulkan
