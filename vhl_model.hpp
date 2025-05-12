#pragma once

#include "vhl_device.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace vhl {
    class VhlModel {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;


            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            Vertex(glm::vec3 pos) : position(pos) { color = glm::vec3{0.0f, 0.0f, 1.0f}; }
            Vertex(glm::vec3 pos, glm::vec3 col) : position(pos), color(col) {}

        };

        VhlModel(VhlDevice& device, const std::vector<Vertex>& vertices);
        ~VhlModel();

        VhlModel(const VhlModel&) = delete;
        VhlModel& operator=(const VhlModel&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);

        VhlDevice& m_VhlDevice;
        VkBuffer m_VertexBuffer;
        VkDeviceMemory m_VertexBufferMemory;
        uint32_t m_VertexCount;
    };
}  // namespace Vhl