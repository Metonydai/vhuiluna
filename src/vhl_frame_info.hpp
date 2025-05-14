#pragma once

#include "vhl_camera.hpp"
#include "vhl_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vhl {
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

