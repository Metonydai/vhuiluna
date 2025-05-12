#include "vhl_renderer.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace vhl {

    VhlRenderer::VhlRenderer(VhlWindow& window, VhlDevice& device)
        : m_VhlWindow{window}, m_VhlDevice{device} 
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    VhlRenderer::~VhlRenderer() { freeCommandBuffers(); }

    void VhlRenderer::recreateSwapChain() 
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
            std::shared_ptr<VhlSwapChain> oldSwapChain = std::move(m_VhlSwapChain);
            m_VhlSwapChain = std::make_unique<VhlSwapChain>(m_VhlDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_VhlSwapChain.get())) 
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void VhlRenderer::createCommandBuffers() 
    {
        m_CommandBuffers.resize(VhlSwapChain::MAX_FRAMES_IN_FLIGHT);

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

    void VhlRenderer::freeCommandBuffers() 
    {
        vkFreeCommandBuffers(
            m_VhlDevice.device(),
            m_VhlDevice.getCommandPool(),
            static_cast<uint32_t>(m_CommandBuffers.size()),
            m_CommandBuffers.data());
        m_CommandBuffers.clear();
    }

    VkCommandBuffer VhlRenderer::beginFrame() 
    {
        assert(!m_IsFrameStarted && "Can't call beginFrame while already in progress");

        auto result = m_VhlSwapChain->acquireNextImage(&m_CurrentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_IsFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void VhlRenderer::endFrame() 
    {
        assert(m_IsFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_VhlSwapChain->submitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_VhlWindow.wasWindowResized()) 
        {
            m_VhlWindow.resetWindowResizedFlag();
            recreateSwapChain();
        } 
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_IsFrameStarted = false;
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VhlSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void VhlRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) 
    {
        assert(m_IsFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_VhlSwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_VhlSwapChain->getFrameBuffer(m_CurrentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_VhlSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_VhlSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_VhlSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, m_VhlSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void VhlRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) 
    {
        assert(m_IsFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }

}  // namespace Vhl