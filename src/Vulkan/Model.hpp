#pragma once

#include "stdafx.hpp"

#include "VulkanDevice.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace OselEngine {
namespace Vulkan {
class Model {
public:
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
	};

	Model(Device&, const std::vector<Vertex>&, const std::vector<uint16_t>&);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void bind(VkCommandBuffer commandBuffer);

	void draw(VkCommandBuffer commandBuffer);

private:
	void createVertexBuffers(const std::vector<Vertex>&);
	void createIndexBuffers(const std::vector<uint16_t>&);

	Device& mDevice;

	VkBuffer mVertexBuffer;
	VkBuffer mIndexBuffer;
	VkDeviceMemory mVertexMemory;
	VkDeviceMemory mIndexMemory;
	uint32_t mVertexCount;
	uint32_t mIndexCount;
};
} //namespace Vulkan
} //namespace PhoenixEngine
