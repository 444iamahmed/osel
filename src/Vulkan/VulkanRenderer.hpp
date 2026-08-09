#pragma once

#include "stdafx.hpp"

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"

#include <memory>
#include <cstdint>
#include <filesystem>

#include "Pipeline.hpp"

namespace OselEngine {
namespace Vulkan {

struct SimplePushConstant {
	glm::mat4 transform;
};

class Renderer {
public:
	Renderer(Window&, Device&);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer &operator=(const Renderer&) = delete;

	void drawFrame();
	void loadModels();

	VkCommandBuffer getCurrentCommandBuffer() const { return mCommandBuffers[mSwapChain->getCurrentFrame()]; }
	VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer&, uint32_t imageIndex) const;
	void recreateSwapChain();

	//Temporary
	void createPipeline();
	void createPushConstantRanges();
	void createPipelineLayout();

	VkPipelineLayout mPipelineLayout;
	std::unique_ptr<Pipeline> mPipeline;

	std::vector<VkPushConstantRange> mPushConstantRanges;

	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<SwapChain> mSwapChain;

	std::unique_ptr<Model> mModel;

	std::vector<VkCommandBuffer> mCommandBuffers;

	uint32_t mCurrentImageIndex;
	uint32_t mTimeFrame{0};
};
}
}
