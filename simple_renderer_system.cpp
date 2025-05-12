#include "simple_renderer_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace vhl 
{
    struct SimplePushConstantData {
        glm::mat4 transform{1.f};
        //glm::vec2 offset;
        alignas(16) glm::vec3 color;
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

        for (auto& obj : gameObjects) 
        {
            obj.transform.rotation.z = glm::mod(obj.transform.rotation.z + 0.0003f, glm::two_pi<float>());
            obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.00015f, glm::two_pi<float>());
        
            SimplePushConstantData push{};
            //push.offset = obj.transform.translation;
            push.color = obj.color;
            push.transform = camera.getProjection() * obj.transform.mat4();
        
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
