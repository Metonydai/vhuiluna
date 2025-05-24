#include "app.hpp"

#include "keyboard_movement_controller.hpp"
#include "vhl_buffer.hpp"
#include "vhl_shadow_map.hpp"
#include "systems/shadow_renderer_system.hpp"
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
            .setMaxSets(4 * VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 * VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 * VhlSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }
      
    HuiApp::~HuiApp() {}

    void HuiApp::run() 
    {

        UniformDataScene uniformDataScene;
        UniformDataOffscreen uniformDataOffscreen;
        UniformBuffer uniformBuffers;
        DescriptorSets descriptorSets;

        // shadow map
        VhlShadowMap shadowMap{m_VhlDevice, 2048};

        uniformBuffers.scene.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        uniformBuffers.offscreen.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        
        for (int i = 0; i < VhlSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            uniformBuffers.scene[i] = std::make_unique<VhlBuffer>(
                m_VhlDevice,
                sizeof(UniformDataScene),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniformBuffers.scene[i]->map();

            uniformBuffers.offscreen[i] = std::make_unique<VhlBuffer>(
                m_VhlDevice,
                sizeof(UniformDataOffscreen),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uniformBuffers.offscreen[i]->map();
        }

        auto globalSetLayout = VhlDescriptorSetLayout::Builder(m_VhlDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        descriptorSets.offscreen.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        descriptorSets.scene.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        descriptorSets.debug.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < VhlSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto sceneBufferInfo = uniformBuffers.scene[i]->descriptorInfo();
            auto imageInfo = shadowMap.getDescriptorImageInfo();

            // Debug display
            VhlDescriptorWriter(*globalSetLayout, *m_GlobalPool)
                .writeBuffer(0, &sceneBufferInfo)        // Binding 0 : Vertex shader uniform buffer
                .writeImage(1, &imageInfo)        // Binding 1 : Fragment shader shadow sampler
                .build(descriptorSets.debug[i]);
            
            // Offscreen shadow map generation
            auto offscreenBufferInfo = uniformBuffers.offscreen[i]->descriptorInfo();
            VhlDescriptorWriter(*globalSetLayout, *m_GlobalPool)
                .writeBuffer(0, &offscreenBufferInfo)        // Binding 0 : Vertex shader uniform buffer
                .build(descriptorSets.offscreen[i]);
            
            // Scene rendering with shadow map applied 
            VhlDescriptorWriter(*globalSetLayout, *m_GlobalPool)
                .writeBuffer(0, &sceneBufferInfo)        // Binding 0 : Vertex shader uniform buffer
                .writeImage(1, &imageInfo)        // Binding 1 : Fragment shader shadow sampler
                .build(descriptorSets.scene[i]);
        }

        //SimpleRenderSystem simpleRenderSystem(
        //    m_VhlDevice, 
        //    m_VhlRenderer.getSwapChainRenderPass(), 
        //    globalSetLayout->getDescriptorSetLayout());

        ShadowRenderSystem shadowRenderSystem(
            m_VhlDevice, 
            shadowMap.getShadowRenderPass(),
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
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if (auto commandBuffer = m_VhlRenderer.beginFrame())
            {
                int frameIndex = m_VhlRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, nullptr, m_GameObjects};
                // update
                // uniformDataScene -> uniformBuffers.scene
                // uniformDataOffscreen -> uniformBuffers.offscreen

                // Matrix from light's point of view
                auto lightPos = m_GameObjects.at(pointLightId).transform.translation;

                glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(shadowMap.lightFOV), 1.0f, shadowMap.zNear, shadowMap.zFar);
                glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.f, 0.5f, 0.f), glm::vec3(0, 1, 0));

                uniformDataOffscreen.depthVP = depthProjectionMatrix * depthViewMatrix;
                uniformBuffers.offscreen[frameIndex]->writeToBuffer(&uniformDataOffscreen);
                uniformBuffers.offscreen[frameIndex]->flush();

                uniformDataScene.projection = camera.getProjection();
                uniformDataScene.view = camera.getView();
                uniformDataScene.inverseView = camera.getInverseView();
                uniformDataScene.depthBiasVP = uniformDataOffscreen.depthVP;
                uniformDataScene.zNear = shadowMap.zNear;
                uniformDataScene.zFar = shadowMap.zFar;
                pointLightSystem.update(frameInfo, uniformDataScene);

                uniformBuffers.scene[frameIndex]->writeToBuffer(&uniformDataScene);
                uniformBuffers.scene[frameIndex]->flush();

                /*
                    First render pass: Generate shadow map by rendering the scene from light's POV
                */

                {
                    VkClearValue clearValues[2];

                    clearValues[0].depthStencil = { 1.0f, 0 };
                    VkRenderPassBeginInfo renderPassBeginInfo{};
                    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    renderPassBeginInfo.renderPass = shadowMap.getShadowRenderPass();
                    renderPassBeginInfo.framebuffer = shadowMap.getShadowFrameBuffer();
                    renderPassBeginInfo.renderArea.extent = shadowMap.getShadowMapExtent();
                    renderPassBeginInfo.clearValueCount = 1;
                    renderPassBeginInfo.pClearValues = clearValues;

                    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                    VkViewport viewport{};
                    viewport.x = 0.0f;
                    viewport.y = 0.0f;
                    viewport.width = static_cast<float>(shadowMap.getShadowMapExtent().width);
                    viewport.height = static_cast<float>(shadowMap.getShadowMapExtent().height);
                    viewport.minDepth = 0.0f;
                    viewport.maxDepth = 1.0f;
                    VkRect2D scissor{{0, 0}, shadowMap.getShadowMapExtent()};
                    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                    // Set depth bias (aka "Polygon offset")
                    // Required to avoid shadow mapping artifacts
                    vkCmdSetDepthBias(
                        commandBuffer,
                        shadowMap.depthBiasConstant,
                        0.0f,
                        shadowMap.depthBiasSlope);
                    
                    shadowRenderSystem.renderShadowMap(frameInfo, descriptorSets);

                    vkCmdEndRenderPass(commandBuffer);
                }

                /*
                    Note: Explicit synchronization is not required between the render pass, as this is done implicit via sub pass dependencies
                */

                /*
                    Second pass: Scene rendering with applied shadow map
                */
                // render
                m_VhlRenderer.beginSwapChainRenderPass(commandBuffer);

                // order here matters
                shadowRenderSystem.renderGameObjects(frameInfo, descriptorSets);
                pointLightSystem.render(frameInfo, descriptorSets);

                m_VhlRenderer.endSwapChainRenderPass(commandBuffer);
                m_VhlRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_VhlDevice.device());
    }

    void HuiApp::loadGameObjects()
    {
        std::shared_ptr<VhlModel> vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/eupho.obj");
        
        auto eupho = VhlGameObject::createGameObject();
        eupho.model = vhlModel;
        eupho.transform.translation = { 0.f, .5f, 0.f };
        eupho.transform.rotation = { -glm::radians(20.0f), 0.f, 0.f};
        eupho.transform.scale = glm::vec3{0.02f};
        eupho.color = { 1.0f, 1.0f, 0.0f };
        m_GameObjects.emplace(eupho.getId(), std::move(eupho));

        vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/quad.obj");
        auto floor = VhlGameObject::createGameObject();
        floor.model = vhlModel;
        floor.transform.translation = { 0.f, 0.5f, 0.f };
        floor.transform.scale = glm::vec3{ 3.0f, 3.0f, 3.0f };
        m_GameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors
        {
            //{ 1.f, .1f, .1f},
            //{.1f, .1f, 1.f},
            //{.1f, 1.f, .1f},
            //{1.f, 1.f, .1f},
            //{.1f, 1.f, 1.f},
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
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-2.f, -2.f, -2.f, 1.f));
            m_GameObjects.emplace(pointLight.getId(), std::move(pointLight));
            //pointLightId = pointLight.getId();
            pointLightId = pointLight.getId();
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