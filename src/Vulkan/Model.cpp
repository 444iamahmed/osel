#include "Model.hpp"

#include <assert.h>

namespace OselEngine::Vulkan {
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> descriptions(2); 
    descriptions[0].binding = 0;
    descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[0].location = 0;
    descriptions[0].offset = offsetof(Vertex, position);

    descriptions[1].binding = 0;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].location = 1;
    descriptions[1].offset = offsetof(Vertex, color);
	return descriptions;
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> descriptions(1);
    
    descriptions[0].binding = 0;
    descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    descriptions[0].stride = sizeof(Vertex);

    return descriptions;
}

Model::Model(Device& inDevice, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) : mDevice{inDevice} {
    createVertexBuffers(vertices);
    createIndexBuffers(indices);
}

Model::~Model() {
    vkDestroyBuffer(mDevice.get(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice.get(), mVertexMemory, nullptr);

    vkDestroyBuffer(mDevice.get(), mIndexBuffer, nullptr);
    vkFreeMemory(mDevice.get(), mIndexMemory, nullptr);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
    spdlog::info("attempting buffer creation");
    mVertexCount = static_cast<uint32_t>(vertices.size());
    assert(mVertexCount >= 3 && "Vertex count must be at least 3");

    VkDeviceSize size = sizeof(vertices[0]) * mVertexCount;

    mDevice.createBuffer(
        size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mVertexBuffer,
        mVertexMemory
    );

    void *data;
    vkMapMemory(mDevice.get(), mVertexMemory, 0, size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    vkUnmapMemory(mDevice.get(), mVertexMemory);
}

void Model::createIndexBuffers(const std::vector<uint16_t>& indices) {
    spdlog::info("attempting buffer creation");
    mIndexCount = static_cast<uint32_t>(indices.size());
    assert(mIndexCount >= 3 && "Index count must be at least 3");

    VkDeviceSize size = sizeof(uint16_t) * mIndexCount;

    mDevice.createBuffer(
        size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mIndexBuffer,
        mIndexMemory
    );

    void *data;
    vkMapMemory(mDevice.get(), mIndexMemory, 0, size, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(size));
    vkUnmapMemory(mDevice.get(), mIndexMemory);
}

void Model::bind(VkCommandBuffer commandBuffer) {
    VkBuffer buffers[] = { mVertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void Model::draw(VkCommandBuffer commandBuffer) {
    vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
}
}


