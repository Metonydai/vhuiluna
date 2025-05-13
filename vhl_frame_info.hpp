#pragma once

#include "vhl_camera.hpp"

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
    };
}

