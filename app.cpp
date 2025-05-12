#include "app.hpp"

#include "simple_renderer_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
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

        while (!m_VhlWindow.shouldClose())
        {
            glfwPollEvents();

            if (auto commandBuffer = m_VhlRenderer.beginFrame())
            {
                m_VhlRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, m_GameObjects);
                m_VhlRenderer.endSwapChainRenderPass(commandBuffer);
                m_VhlRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(m_VhlDevice.device());
    }

    // temporary helper function, creates a 1x1x1 cube centered at offset
    std::shared_ptr<VhlModel> createCubeModel(VhlDevice& device, glm::vec3 offset) {
        std::vector<VhlModel::Vertex> vertices
        {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        
            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        
            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        
            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        
            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        
            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        };

        for (auto& v : vertices) 
        {
            v.position += offset;
        }
        return std::make_shared<VhlModel>(device, vertices);
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

        for (auto& vert : vertices)
        {
            vert.position.x /= aspectRatio;
        }

        auto vhlModel = std::make_shared<VhlModel>(m_VhlDevice, vertices);
        auto triangle = VhlGameObject::createGameObject();
        triangle.model = vhlModel;
        triangle.color = glm::vec3{ 0.0f, 1.0f, 1.0f };
        triangle.transform.translation = glm::vec3{ 0.0f };
        triangle.transform.scale = glm::vec3{ 0.5f };

        m_GameObjects.push_back(std::move(triangle));
        */

        std::shared_ptr<VhlModel> vhlModel = createCubeModel(m_VhlDevice, {.0f, .0f, .0f});
        auto cube = VhlGameObject::createGameObject();
        cube.model = vhlModel;
        cube.transform.translation = { .0f, .0f, .5f };
        cube.transform.scale = { .5f, .5f, .5f };
    
        m_GameObjects.push_back(std::move(cube));

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