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
        glm::mat4 transform{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(VhlDevice& device, VkRenderPass renderPass) : m_VhlDevice(device) 
    {
        createPipelineLayout();
        createPipeline(renderPass);
    }
      
    SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(m_VhlDevice.device(), m_PipelineLayout, nullptr); }

    void SimpleRenderSystem::createPipelineLayout() 
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
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
      

    void SimpleRenderSystem::renderGameObjects(
        VkCommandBuffer commandBuffer, 
        std::vector<VhlGameObject>& gameObjects,
        const VhlCamera& camera)
    {
        m_VhlPipeline->bind(commandBuffer);

        auto projectView = camera.getProjection() * camera.getView();

        for (auto& obj : gameObjects) 
        {
            SimplePushConstantData push{};
            push.transform = projectView * obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();
        
            vkCmdPushConstants(
                commandBuffer,
                m_PipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
          }
    }

}
