#pragma once

#include "vhl_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>

namespace vhl {
    class VhlModel {
    public:
        struct Vertex 
        {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            Vertex() {}
            Vertex(glm::vec3 pos) : position(pos) { color = glm::vec3{0.0f, 0.0f, 1.0f}; }
            Vertex(glm::vec3 pos, glm::vec3 col) : position(pos), color(col) {}

            bool operator==(const Vertex& other) const
            {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };

        VhlModel(VhlDevice& device, const VhlModel::Builder& builder);
        ~VhlModel();

        VhlModel(const VhlModel&) = delete;
        VhlModel& operator=(const VhlModel&) = delete;

        static std::unique_ptr<VhlModel> createModelFromFile(VhlDevice& device, const std::string& filepath);

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        VhlDevice& m_VhlDevice;

        VkBuffer m_VertexBuffer;
        VkDeviceMemory m_VertexBufferMemory;
        uint32_t m_VertexCount;
        
        bool m_HasIndexBuffer = false;
        VkBuffer m_IndexBuffer;
        VkDeviceMemory m_IndexBufferMemory;
        uint32_t m_IndexCount;
    };
}  // namespace Vhl