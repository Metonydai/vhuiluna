#include "simple_renderer_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace vhl 
{
    struct SimplePushConstantData 
    {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(VhlDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_VhlDevice(device) 
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }
      
    SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(m_VhlDevice.device(), m_PipelineLayout, nullptr); }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_VhlDevice.device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) !=
            VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    } 

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) 
    {
        PipelineConfigInfo pipelineConfig{};
        VhlPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = m_PipelineLayout;
        m_VhlPipeline = std::make_unique<VhlPipeline>(
            m_VhlDevice,
            "shaders/shader.vert.spv",
            "shaders/shader.frag.spv",
            pipelineConfig);
    }
      

    void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo)
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

        for (auto& kv : frameInfo.gameObjects) 
        {
            auto& obj = kv.second;
            if (obj.model == nullptr) continue;
            SimplePushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();
        
            vkCmdPushConstants(
                frameInfo.commandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

}
