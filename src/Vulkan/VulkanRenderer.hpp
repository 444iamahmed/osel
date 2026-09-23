#pragma once

#include "stdafx.hpp"

#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "SwapChain.hpp"
#include "Model.hpp"
#include "Texture.hpp"

#include <memory>
#include <cstdint>

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
	Renderer& operator=(const Renderer&) = delete;

	void drawFrame();
	void loadModels();

	VkCommandBuffer getCurrentCommandBuffer() const { return mCommandBuffers[mSwapChain->getCurrentFrame()]; }
	VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

	void updateDescriptorSets(const Texture&);

private:
	void createCommandBuffers();
	void freeCommandBuffers();
	void freeDescriptorSets();
	void recordCommandBuffer(VkCommandBuffer&, uint32_t imageIndex) const;
	void recreateSwapChain();

	// Temporary
	void createPipeline();
	void createPushConstantRanges();
	void createDescriptorSetLayouts();
	void allocateDescriptorSets();
	void createPipelineLayout();

	VkPipelineLayout mPipelineLayout;
	std::unique_ptr<Pipeline> mPipeline;

	std::vector<VkPushConstantRange> mPushConstantRanges;
	std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;

	std::vector<VkDescriptorSet> mDescriptorSets;

	Window& mWindow;
	Device& mDevice;
	std::unique_ptr<SwapChain> mSwapChain;

	std::unique_ptr<Model> mModel;

	std::vector<VkCommandBuffer> mCommandBuffers;

	uint32_t mCurrentImageIndex;
	uint32_t mTimeFrame{0};
};
} // namespace Vulkan
} // namespace OselEngine
