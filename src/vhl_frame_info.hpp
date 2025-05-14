#pragma once

#include "vhl_camera.hpp"
#include "vhl_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vhl {

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    struct GlobalUBO
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo 
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        VhlCamera& camera;
        VkDescriptorSet globalDescriptorSet;
        VhlGameObject::Map& gameObjects;
    };
}

