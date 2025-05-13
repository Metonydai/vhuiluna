#include "vhl_model.hpp"
#include "vhl_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std
{
    template <>
    struct hash<vhl::VhlModel::Vertex>
    {
        size_t operator()(vhl::VhlModel::Vertex const& vertex) const
        {
            size_t seed = 0;
            vhl::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
} // namespace std



namespace vhl {

    VhlModel::VhlModel(VhlDevice& device, const VhlModel::Builder& builder) : m_VhlDevice(device) 
    {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    VhlModel::~VhlModel() 
    {
        vkDestroyBuffer(m_VhlDevice.device(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_VhlDevice.device(), m_VertexBufferMemory, nullptr);
        if (m_HasIndexBuffer)
        {
            vkDestroyBuffer(m_VhlDevice.device(), m_IndexBuffer, nullptr);
            vkFreeMemory(m_VhlDevice.device(), m_IndexBufferMemory, nullptr);
        }
    }

    std::unique_ptr<VhlModel> VhlModel::createModelFromFile(VhlDevice& device, const std::string& filepath)
    {
        Builder builder{};
        builder.loadModel(filepath);
        
        return std::make_unique<VhlModel>(device, builder);
    }

    void VhlModel::createVertexBuffers(const std::vector<Vertex>& vertices) 
    {
        m_VertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_VhlDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void *data;
        vkMapMemory(m_VhlDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_VhlDevice.device(), stagingBufferMemory);

        m_VhlDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer,
            m_VertexBufferMemory);

        m_VhlDevice.copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(m_VhlDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_VhlDevice.device(), stagingBufferMemory, nullptr);
    }

    void VhlModel::createIndexBuffers(const std::vector<uint32_t>& indices) 
    {
        m_IndexCount = static_cast<uint32_t>(indices.size());
        m_HasIndexBuffer = m_IndexCount > 0;

        if (!m_HasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_VhlDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory);

        void *data;
        vkMapMemory(m_VhlDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_VhlDevice.device(), stagingBufferMemory);

        m_VhlDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer,
            m_IndexBufferMemory);

        m_VhlDevice.copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_VhlDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(m_VhlDevice.device(), stagingBufferMemory, nullptr);
    }

    void VhlModel::draw(VkCommandBuffer commandBuffer) 
    {
        if (m_HasIndexBuffer) 
            vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
        else
            vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }

    void VhlModel::bind(VkCommandBuffer commandBuffer) 
    {
        VkBuffer buffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_HasIndexBuffer)
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
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
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        attributeDescriptions.reserve(4);

        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

        return attributeDescriptions;
    }

    void VhlModel::Builder::loadModel(const std::string& filepath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
      
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) 
        {
            throw std::runtime_error(warn + err);
        }
      
        vertices.clear();
        indices.clear();
      
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};
        
                if (index.vertex_index >= 0) 
                {
                    vertex.position = 
                    {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
            
                    vertex.color = 
                    {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }
        
                if (index.normal_index >= 0) 
                {
                    vertex.normal = 
                    {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }
        
                if (index.texcoord_index >= 0) 
                {
                    vertex.uv = 
                    {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }
        
                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

}  // namespace vhl 