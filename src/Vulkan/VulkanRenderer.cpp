#include "VulkanRenderer.hpp"

#include "VulkanDevice.hpp"
#include "Model.hpp"
#include "../Utils.hpp"

#include <cstdint>
#include <fstream>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "Pipeline.hpp"

namespace OselEngine {
namespace Vulkan {
Renderer::Renderer(Window& window, Device& device)
: mWindow{window}, mDevice{device} {
	recreateSwapChain();
	createCommandBuffers();
	createPushConstantRanges();
	createPipelineLayout();
	createPipeline();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::drawFrame() {
	VkResult result = mSwapChain->acquireNextImage(&mCurrentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		spdlog::warn("trying to recreate swapchain");
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swapchain image");
	}

	VkCommandBuffer currentCommandBuffer = getCurrentCommandBuffer();

	vkResetCommandBuffer(currentCommandBuffer, 0);
	recordCommandBuffer(currentCommandBuffer, mCurrentImageIndex);

	mSwapChain->submitCommandBuffer(&currentCommandBuffer, &mCurrentImageIndex);

	mTimeFrame++;
}


void Renderer::recreateSwapChain() {
	VkExtent2D extent = mWindow.getExtent();
	while (extent.width == 0 || extent.height == 0) {
		extent = mWindow.getExtent();
		glfwWaitEvents();
	}

	spdlog::warn("{}, {}", extent.width, extent.height);
	vkDeviceWaitIdle(mDevice.get());

	if (mSwapChain == nullptr) {
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent);
	} else {
		std::shared_ptr<SwapChain> oldSwapChain = std::move(mSwapChain);
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent, oldSwapChain);
	}
}

void Renderer::createCommandBuffers() {
	mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mDevice.getCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice.get(), &allocInfo, mCommandBuffers.data()) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
	spdlog::error("Command buffer allocated");
}

void Renderer::freeCommandBuffers() {
	vkFreeCommandBuffers(
		mDevice.get(),
		mDevice.getCommandPool(),
		static_cast<uint32_t>(mCommandBuffers.size()),
		mCommandBuffers.data());
  mCommandBuffers.clear();
}

void Renderer::recordCommandBuffer(VkCommandBuffer& commandBuffer,
								 uint32_t imageIndex) const {
	spdlog::info("recording command buffer");
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;				   // Optional
	beginInfo.pInheritanceInfo = nullptr;  // Optional
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	auto swapChainExtent = mSwapChain->getSwapChainExtent();

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mSwapChain->getRenderPass();
	renderPassInfo.framebuffer = mSwapChain->getFrameBuffer(imageIndex);
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = mSwapChain->getSwapChainExtent();
	VkClearValue clearColor = {{{0.5f, 0.5f, 0.5f, 1.0f}}};
	VkClearValue clearDepth = {.depthStencil = {1.0, 0}};

	VkClearValue clearValues[] = {clearColor, clearDepth};
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
						 VK_SUBPASS_CONTENTS_INLINE);
	mPipeline->bind(commandBuffer);
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	spdlog::info("drawing model");
	mModel->bind(commandBuffer);

	//make a model view matrix for rendering the object
	//camera position
	glm::vec3 camPos = { 0.f,0.f,-2.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;
	//model rotation
	glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(mTimeFrame * 0.01f), glm::vec3(0, 1, 0));

	//calculate final mesh matrix
	glm::mat4 mesh_matrix = projection * view * model;

	SimplePushConstant constants;
	constants.transform = mesh_matrix;

	vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(SimplePushConstant), &constants);

	mModel->draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Renderer::loadModels() {
	spdlog::info("attempting model loading");
	std::vector<Model::Vertex> vertices {
		{{0.0, -0.5, .0}, {1.0, 0.0, 0.0}},
		{{0.5, 0.5, .0}, {0.0, 1.0, 0.0}},
		{{-0.5, 0.5, .0}, {0.0, 0.0, 1.0}},
		{{-0., 0.0, 0.5}, {1.0, 0.0, 1.0}}
	};

	std::vector<uint16_t> indices {
		1,2,3,
		3,2,0,
		3,0,1,
		2,1,0
	};

	spdlog::info("successfully created vertices");

	mModel = std::make_unique<Model>(mDevice, vertices, indices);
	spdlog::info("successfully created model");
}
	
void Renderer::createPipeline() {
	PipelineConfigInfo pipelineConfigInfo{};

	Pipeline::defaultPipelineConfigInfo(pipelineConfigInfo);
	pipelineConfigInfo.mRenderPass = getSwapChainRenderPass();
	pipelineConfigInfo.mPipelineLayout = mPipelineLayout;

	mPipeline =  std::make_unique<Pipeline>(
		mDevice,
		"shaders/out/shader.vert.spv",
		"shaders/out/shader.frag.spv",
		pipelineConfigInfo
		);
}

void Renderer::createPushConstantRanges() {
	mPushConstantRanges.reserve(1);

	mPushConstantRanges.push_back({
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(SimplePushConstant)
	});
}

void Renderer::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			// Optional

	pipelineLayoutInfo.pPushConstantRanges = mPushConstantRanges.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(mPushConstantRanges.size());	// Optional

	if (vkCreatePipelineLayout(mDevice.get(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
} // namespace Vulkan
} // namespace PhoenixEngine
