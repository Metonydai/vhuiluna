#pragma once

#include <vulkan/vulkan.h>

#include "vhl_device.hpp"

namespace vhl
{
    class VhlShadowMap
    {
	public:
		bool displayShadowMap = false;
		static constexpr bool filterPCF = true;

		// Keep depth range as small as possible
		// for better shadow map precision
		float zNear = 1.0f;
		float zFar = 8.0f;

		// Depth bias (and slope) are used to avoid shadowing artifacts
		// Constant depth bias factor (always applied)
		float depthBiasConstant = 1.25f;
		// Slope depth bias factor, applied depending on polygon's slope
		float depthBiasSlope = 1.75f;

        float lightFOV = 50.0f;

        	// 16 bits of depth is enough for such a small scene
	const VkFormat offscreenDepthFormat{ VK_FORMAT_D16_UNORM };
    public:
        enum class SubPassesShadow
        {
            SUBPASS_SHADOW = 0,
            NUMBER_OF_SUBPASSES
        };

        enum class ShadowRenderTargets
        {
            ATTACHMENT_DEPTH = 0,
            NUMBER_OF_ATTACHMENTS
        };

    public:
        VhlShadowMap(VhlDevice& device, int width);
        ~VhlShadowMap();

        VhlShadowMap(const VhlShadowMap&) = delete;
        VhlShadowMap& operator=(const VhlShadowMap&) = delete;

        VkFramebuffer getShadowFrameBuffer() { return m_ShadowFramebuffer; }
        VkRenderPass getShadowRenderPass() { return m_ShadowRenderPass; }
        VkExtent2D getShadowMapExtent() { return m_ShadowMapExtent; }
        const VkDescriptorImageInfo& getDescriptorImageInfo() const { return m_DescriptorImageInfo; }

    private:
        void createShadowDepthResources();
        void createShadowRenderPass();
        void createShadowFramebuffer();

    private:
        VkFormat m_DepthFormat{VkFormat::VK_FORMAT_UNDEFINED};
        VhlDevice& m_Device;

        VkExtent2D m_ShadowMapExtent{};
        VkFramebuffer m_ShadowFramebuffer{nullptr};
        VkRenderPass m_ShadowRenderPass{nullptr};

        VkImage m_ShadowDepthImage{nullptr};
        VkImageLayout m_ImageLayout{};
        VkImageView m_ShadowDepthImageView{nullptr};
        VkDeviceMemory m_ShadowDepthImageMemory{nullptr};
        VkSampler m_ShadowDepthSampler{nullptr};

        VkDescriptorImageInfo m_DescriptorImageInfo{};
    };
} // namespace vhl