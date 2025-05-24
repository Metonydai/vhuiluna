#pragma once

#include "vhl_camera.hpp"
#include "vhl_game_object.hpp"
#include "vhl_buffer.hpp"

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
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f}; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct UniformDataScene {
        glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
        glm::mat4 inverseView{ 1.f };
        glm::mat4 model{ 1.f };
        glm::mat4 depthBiasVP{ 1.f };
        glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .20f }; // w is intensity
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
        // Used for depth map visualization
        float zNear;
        float zFar;
    };
    
	struct UniformDataOffscreen {
		glm::mat4 depthVP;
	};

    struct UniformBuffer{
		std::vector<std::unique_ptr<VhlBuffer>> scene;
		std::vector<std::unique_ptr<VhlBuffer>> offscreen;
	};

    struct DescriptorSets{
		std::vector<VkDescriptorSet> offscreen{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> scene{ VK_NULL_HANDLE };
		std::vector<VkDescriptorSet> debug{ VK_NULL_HANDLE };
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

