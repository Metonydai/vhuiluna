#include "app.hpp"

#include "keyboard_movement_controller.hpp"
#include "simple_renderer_system.hpp"

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
        loadGameObjects();
    }
      
    HuiApp::~HuiApp() {}

    void HuiApp::run() 
    {
        SimpleRenderSystem simpleRenderSystem(m_VhlDevice, m_VhlRenderer.getSwapChainRenderPass());
        VhlCamera camera{};
        //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
        //camera.setViewTarget(glm::vec3(-2.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewerObject = VhlGameObject::createGameObject();
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
                m_VhlRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, m_GameObjects, camera);
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
        flatVase.transform.translation = { -0.5f, .5f, 2.5f };
        flatVase.transform.scale = glm::vec3{ 3.0f, 1.5f, 3.0f };
        
        m_GameObjects.push_back(std::move(flatVase));

        vhlModel = VhlModel::createModelFromFile(m_VhlDevice, "models/smooth_vase.obj");
        auto smoothVase = VhlGameObject::createGameObject();
        smoothVase.model = vhlModel;
        smoothVase.transform.translation = { 0.5f, .5f, 2.5f };
        smoothVase.transform.scale = glm::vec3{ 3.0f, 1.5f, 3.0f };
        
        m_GameObjects.push_back(std::move(smoothVase));
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