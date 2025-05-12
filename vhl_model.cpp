#include "vhl_model.hpp"

// std
#include <cassert>
#include <cstring>

namespace vhl {

    VhlModel::VhlModel(VhlDevice& device, const std::vector<Vertex>& vertices) : m_VhlDevice(device) 
    {
        createVertexBuffers(vertices);
    }

    VhlModel::~VhlModel() 
    {
        vkDestroyBuffer(m_VhlDevice.device(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_VhlDevice.device(), m_VertexBufferMemory, nullptr);
    }

    void VhlModel::createVertexBuffers(const std::vector<Vertex>& vertices) 
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        m_VhlDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_VertexBuffer,
            m_VertexBufferMemory);

        void *data;
        vkMapMemory(m_VhlDevice.device(), m_VertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_VhlDevice.device(), m_VertexBufferMemory);
    }

    void VhlModel::draw(VkCommandBuffer commandBuffer) 
    {
        vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }

    void VhlModel::bind(VkCommandBuffer commandBuffer) 
    {
        VkBuffer buffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }

    std::vector<VkVertexInputBindingDescription> VhlModel::Vertex::getBindingDescriptions() 
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VhlModel::Vertex::getAttributeDescriptions() 
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }

}  // namespace vhl 