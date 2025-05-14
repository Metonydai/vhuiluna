#include "app.hpp"

#include "keyboard_movement_controller.hpp"
#include "vhl_buffer.hpp"
#include "systems/simple_renderer_system.hpp"
#include "systems/point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <chrono>
#include <array>

namespace vhl 
{
    HuiApp::HuiApp() 
    {
        m_GlobalPool = VhlDescriptorPool::Builder(m_VhlDevice)
            .setMaxSets(VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }
      
    HuiApp::~HuiApp() {}

    void HuiApp::run() 
    {
        std::vector<std::unique_ptr<VhlBuffer>> uboBuffers(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<VhlBuffer>(
                m_VhlDevice,
                sizeof(GlobalUBO),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = VhlDescriptorSetLayout::Builder(m_VhlDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            VhlDescriptorWriter(*globalSetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem(
            m_VhlDevice, 
            m_VhlRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout());

        PointLightSystem pointLightSystem(
            m_VhlDevice, 
            m_VhlRenderer.getSwapChainRenderPass(), 
            globalSetLayout->getDescriptorSetLayout());

        VhlCamera camera{};
        //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
        //camera.setViewTarget(glm::vec3(-2.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = VhlGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!m_VhlWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(m_VhlWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = m_VhlRenderer.getAspectRatio();
            //float a = 4.0;
            //camera.setOrthographicProjection(-aspect*a, aspect*a, -a, a, -a, a);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

            if (auto commandBuffer = m_VhlRenderer.beginFrame())
            {
                int frameIndex = m_VhlRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], m_GameObjects};
                // update
                GlobalUBO ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                m_VhlRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                m_VhlRenderer.endSwapChainRenderPass(commandBuffer);
                m_VhlRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_VhlDevice.device());
    }

    void HuiApp::loadGameObjects()
    {
        /*
        float aspectRatio = (float)HuiApp::WIDTH / HuiApp::HEIGHT;
        std::vector<VhlModel::Vertex> vertices {
            {{ 0.0f, -1.1546f, 0.f }, { 1.0f, 0.0f, 0.0f }},
            {{-1.0f,  0.5773f, 0.f }, { 0.0f, 1.0f, 0.0f }},
            {{ 1.0f,  0.5773f, 0.f }, { 0.0f, 0.0f, 1.0f }}
        };

        //std::vector<VhlModel::Vertex> vertices {};
        //createFractal(vertices, 6, { 0.0f, -1.1546f, 0.f }, {-1.0f,  0.5773f, 0.f }, { 1.0f,  0.5773f, 0.f } );

        auto vhlModel = std::make_shared<VhlModel>(m_VhlDevice, vertices);
        auto triangle = VhlGameObject::createGameObject();
        triangle.model = vhlModel;
        triangle.color = glm::vec3{ 0.0f, 1.0f, 1.0f };
        triangle.transform.translation = glm::vec3{ 0.0f };
        triangle.transform.scale = glm::vec3{ 0.5f };

        m_GameObjects.push_back(std::move(triangle));
        */

        std::shared_ptr<VhlModel> vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/flat_vase.obj");

        auto flatVase = VhlGameObject::createGameObject();
        flatVase.model = vhlModel;
        flatVase.transform.translation = { -0.5f, .5f, 0.f };
        flatVase.transform.scale = glm::vec3{ 3.0f, 1.5f, 3.0f };
        
        m_GameObjects.emplace(flatVase.getId(), std::move(flatVase));

        vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/smooth_vase.obj");
        auto smoothVase = VhlGameObject::createGameObject();
        smoothVase.model = vhlModel;
        smoothVase.transform.translation = { 0.5f, .5f, 0.f };
        smoothVase.transform.scale = glm::vec3{ 3.0f, 1.5f, 3.0f };
       
        m_GameObjects.emplace(smoothVase.getId(), std::move(smoothVase));


        vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/quad.obj");
        auto floor = VhlGameObject::createGameObject();
        floor.model = vhlModel;
        floor.transform.translation = { 0.f, 0.5f, 0.f };
        floor.transform.scale = glm::vec3{ 3.0f, 1.0f, 3.0f };
        
        m_GameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors
        {
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  
        };

        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = VhlGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f), 
                i * glm::two_pi<float>() / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            m_GameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

    }

    void HuiApp::createFractal(std::vector<VhlModel::Vertex>& vertices, int level, glm::vec3 top, glm::vec3 left, glm::vec3 right)
    {
        if (level <= 0)
        {
            vertices.emplace_back(top);
            vertices.emplace_back(left);
            vertices.emplace_back(right);
            return;
        }
        glm::vec3 leftMid = 0.5f * (top + left);
        glm::vec3 rightMid = 0.5f * (top + right);
        glm::vec3 botMid = 0.5f * (left + right);
    
        createFractal(vertices, level-1, top, leftMid, rightMid);
        createFractal(vertices, level-1, leftMid, left, botMid);
        createFractal(vertices, level-1, rightMid, botMid, right);
    }

}