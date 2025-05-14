#pragma once

#include "vhl_device.hpp"
#include "vhl_swap_chain.hpp"
#include "vhl_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace vhl {
    class VhlRenderer {
    public:
        VhlRenderer(VhlWindow& window, VhlDevice& device);
        ~VhlRenderer();

        VhlRenderer(const VhlRenderer&) = delete;
        VhlRenderer& operator=(const VhlRenderer&) = delete;

        VkRenderPass getSwapChainRenderPass() const { return m_VhlSwapChain->getRenderPass(); }
        float getAspectRatio() const { return m_VhlSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return m_IsFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const 
        {
            assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
            return m_CommandBuffers[m_CurrentFrameIndex];
        }

        int getFrameIndex() const 
        {
            assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
            return m_CurrentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        VhlWindow& m_VhlWindow;
        VhlDevice& m_VhlDevice;
        std::unique_ptr<VhlSwapChain> m_VhlSwapChain;
        std::vector<VkCommandBuffer> m_CommandBuffers;

        uint32_t m_CurrentImageIndex;
        int m_CurrentFrameIndex{0};
        bool m_IsFrameStarted{false};
    };
}  // namespace vhl