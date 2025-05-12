#include "app.hpp"

#include <stdexcept>
#include <array>

namespace vhl 
{
    HuiApp::HuiApp() 
    {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }
      
    HuiApp::~HuiApp() { vkDestroyPipelineLayout(m_VhlDevice.device(), m_PipelineLayout, nullptr); }

    void HuiApp::run() 
    {
        while (!m_VhlWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(m_VhlDevice.device());
    }

    void HuiApp::loadModels()
    {
        float aspectRatio = (float)HuiApp::WIDTH / HuiApp::HEIGHT;
        std::vector<VhlModel::Vertex> vertices {
            {{ 0.0f, -0.850f }, { 1.0f, 0.0f, 0.0f }},
            {{-1.0f,  0.882f }, { 0.0f, 1.0f, 0.0f }},
            {{ 1.0f,  0.882f }, { 0.0f, 0.0f, 1.0f }}
        };

        //std::vector<VhlModel::Vertex> vertices {};
        //createFractal(vertices, 6, { 0.0f, -0.85f }, {-1.0f,  0.882f }, { 1.0f,  0.882f } );

        for (auto& vert : vertices)
        {
            vert.position.x /= aspectRatio;
        }

        m_VhlModel = std::make_unique<VhlModel>(m_VhlDevice, vertices);
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


    void HuiApp::createPipelineLayout() 
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_VhlDevice.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) !=
            VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    } 

    void HuiApp::createPipeline() 
    {
        PipelineConfigInfo pipelineConfig{};
        VhlPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_VhlSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        m_VhlPipeline = std::make_unique<VhlPipeline>(
            m_VhlDevice,
            "shaders/shader.vert.spv",
            "shaders/shader.frag.spv",
            pipelineConfig);
    }
      
    void HuiApp::recreateSwapChain()
    {
        auto extent = m_VhlWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) 
        {
            extent = m_VhlWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_VhlDevice.device());
      
        if (m_VhlSwapChain == nullptr) 
        {
            m_VhlSwapChain = std::make_unique<VhlSwapChain>(m_VhlDevice, extent);
        } 
        else 
        {
            m_VhlSwapChain = std::make_unique<VhlSwapChain>(m_VhlDevice, extent, std::move(m_VhlSwapChain));
            if (m_VhlSwapChain->imageCount() != m_CommandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
      
        createPipeline();
    }

    void HuiApp::createCommandBuffers() 
    {
        m_CommandBuffers.resize(m_VhlSwapChain->imageCount());
      
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_VhlDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());
      
        if (vkAllocateCommandBuffers(m_VhlDevice.device(), &allocInfo, m_CommandBuffers.data()) !=
            VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void HuiApp::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
            m_VhlDevice.device(),
            m_VhlDevice.getCommandPool(),
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());
        m_CommandBuffers.clear();
    }


    void HuiApp::recordCommandBuffer(int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
        if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
    
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_VhlSwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_VhlSwapChain->getFrameBuffer(imageIndex);
    
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_VhlSwapChain->getSwapChainExtent();
    
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    
        vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_VhlSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_VhlSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_VhlSwapChain->getSwapChainExtent() };
        vkCmdSetViewport(m_CommandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(m_CommandBuffers[imageIndex], 0, 1, &scissor);

        m_VhlPipeline->bind(m_CommandBuffers[imageIndex]);
        m_VhlModel->bind(m_CommandBuffers[imageIndex]);
        m_VhlModel->draw(m_CommandBuffers[imageIndex]);
    
        vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
        if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record command buffer!");
        }

    }

    void HuiApp::drawFrame() 
    {
        uint32_t imageIndex;
        auto result = m_VhlSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        recordCommandBuffer(imageIndex);

        result = m_VhlSwapChain->submitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_VhlWindow.wasWindowResized()) 
        {
            m_VhlWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}