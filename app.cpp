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

    void HuiApp::loadGameObjects()
    {
        float aspectRatio = (float)HuiApp::WIDTH / HuiApp::HEIGHT;
        std::vector<VhlModel::Vertex> vertices {
            {{ 0.0f, -1.1546f }, { 1.0f, 0.0f, 0.0f }},
            {{-1.0f,  0.5773f }, { 0.0f, 1.0f, 0.0f }},
            {{ 1.0f,  0.5773f }, { 0.0f, 0.0f, 1.0f }}
        };

        //std::vector<VhlModel::Vertex> vertices {};
        //createFractal(vertices, 6, { 0.0f, -0.85f }, {-1.0f,  0.882f }, { 1.0f,  0.882f } );

        for (auto& vert : vertices)
        {
            vert.position.x /= aspectRatio;
        }

        auto vhlModel = std::make_shared<VhlModel>(m_VhlDevice, vertices);
        auto triangle = VhlGameObject::createGameObject();
        triangle.model = vhlModel;
        triangle.color = glm::vec3{0.0f, 1.0f, 1.0f};
        triangle.transform2d.translation = glm::vec2{ 0.0f, 0.0f };
        triangle.transform2d.scale = glm::vec2{ 0.5f };
        //triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

        m_GameObjects.push_back(std::move(triangle));
    }

    void HuiApp::createFractal(std::vector<VhlModel::Vertex>& vertices, int level, glm::vec2 top, glm::vec2 left, glm::vec2 right)
    {
        if (level <= 0)
        {
            vertices.emplace_back(top);
            vertices.emplace_back(left);
            vertices.emplace_back(right);
            return;
        }
        glm::vec2 leftMid = 0.5f * (top + left);
        glm::vec2 rightMid = 0.5f * (top + right);
        glm::vec2 botMid = 0.5f * (left + right);
    
        createFractal(vertices, level-1, top, leftMid, rightMid);
        createFractal(vertices, level-1, leftMid, left, botMid);
        createFractal(vertices, level-1, rightMid, botMid, right);
    }

}