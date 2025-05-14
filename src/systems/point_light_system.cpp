#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace vhl 
{
    PointLightSystem::PointLightSystem(VhlDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_VhlDevice(device) 
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
      
    PointLightSystem::~PointLightSystem() { vkDestroyPipelineLayout(m_VhlDevice.device(), m_PipelineLayout, nullptr); }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
    {
        //VkPushConstantRange pushConstantRange{};
        //pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        //pushConstantRange.offset = 0;
        //pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_VhlDevice.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) !=
            VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    } 

    void PointLightSystem::createPipeline(VkRenderPass renderPass) 
    {
        PipelineConfigInfo pipelineConfig{};
        VhlPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        m_VhlPipeline = std::make_unique<VhlPipeline>(
            m_VhlDevice,
            "shaders/point_light.vert.spv",
            "shaders/point_light.frag.spv",
            pipelineConfig);
    }
      

    void PointLightSystem::render(FrameInfo& frameInfo)
    {
        m_VhlPipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }

}
