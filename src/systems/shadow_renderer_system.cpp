#include "vhl_shadow_map.hpp"
#include "shadow_renderer_system.hpp"
#include "pushConstantData.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>
#include <algorithm>

namespace vhl 
{
    ShadowRenderSystem::ShadowRenderSystem(VhlDevice& device, VkRenderPass shadowPass, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : m_VhlDevice(device) 
    {
        createPipelineLayout(globalSetLayout);
        createPipelines(shadowPass, renderPass);
    }
      
    ShadowRenderSystem::~ShadowRenderSystem()
    { 
        for (auto& shaderModule : m_ShaderModules)
        {
            vkDestroyShaderModule(m_VhlDevice.device(), shaderModule, nullptr);
        }

        vkDestroyPipelineLayout(m_VhlDevice.device(), m_PipelineLayout, nullptr); 
        vkDestroyPipeline(m_VhlDevice.device(), m_Pipelines.debug, nullptr); 
        vkDestroyPipeline(m_VhlDevice.device(), m_Pipelines.offscreen, nullptr); 
        vkDestroyPipeline(m_VhlDevice.device(), m_Pipelines.sceneShadow, nullptr); 
        vkDestroyPipeline(m_VhlDevice.device(), m_Pipelines.sceneShadowPCF, nullptr); 
    }

    void ShadowRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) 
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

    void ShadowRenderSystem::createPipelines(VkRenderPass shadowPass, VkRenderPass renderPass) 
    {
        PipelineConfigInfo pipelineConfig{};
        VhlPipeline::defaultPipelineConfigInfo(pipelineConfig);
        //VhlPipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.pipelineLayout = m_PipelineLayout;

        auto& bindingDescriptions = pipelineConfig.bindingDescriptions;
        auto& attributeDescriptions = pipelineConfig.attributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &pipelineConfig.colorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
        colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
        colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
        colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
      
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size()); ;
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineConfig.inputAssemblyInfo;
        pipelineInfo.pViewportState = &pipelineConfig.viewportInfo;
        pipelineInfo.pRasterizationState = &pipelineConfig.rasterizationInfo;
        pipelineInfo.pMultisampleState = &pipelineConfig.multisampleInfo;
        pipelineInfo.pColorBlendState = &colorBlendInfo;
        pipelineInfo.pDepthStencilState = &pipelineConfig.depthStencilInfo;
        pipelineInfo.pDynamicState = &pipelineConfig.dynamicStateInfo;
        
        pipelineInfo.layout = pipelineConfig.pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
      
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// Shadow mapping debug quad display
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		// Empty vertex input state
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;

		pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		shaderStages[0] = loadShader("shaders/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader("shaders/quad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

        if (vkCreateGraphicsPipelines( m_VhlDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipelines.debug) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        } 

  		// Scene rendering with shadows applied
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

        shaderStages[0] = loadShader("shaders/scene.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaderStages[1] = loadShader("shaders/scene.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

		// Use specialization constants to select between horizontal and vertical blur
		uint32_t enablePCF = 0;
		VkSpecializationMapEntry specializationMapEntry{};
        specializationMapEntry.constantID = 0;
        specializationMapEntry.offset = 0;
        specializationMapEntry.size = sizeof(uint32_t);

		VkSpecializationInfo specializationInfo{};
        specializationInfo.mapEntryCount = 1;
        specializationInfo.pMapEntries = &specializationMapEntry;
        specializationInfo.dataSize = sizeof(uint32_t);
        specializationInfo.pData = &enablePCF;

        shaderStages[1].pSpecializationInfo = &specializationInfo;

		// No filtering
        if (vkCreateGraphicsPipelines( m_VhlDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipelines.sceneShadow) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        } 
		// PCF filtering
		enablePCF = 1;
        if (vkCreateGraphicsPipelines( m_VhlDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipelines.sceneShadowPCF) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        } 

		// Offscreen pipeline (vertex shader only)
		shaderStages[0] = loadShader("shaders/offscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		pipelineInfo.stageCount = 1;
		// No blend attachment states (no color attachments used)
		colorBlendInfo.attachmentCount = 0;
		// Disable culling, so all faces contribute to shadows
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		// Enable depth bias
        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_TRUE;
		// Add depth bias to dynamic state, so we can change it at runtime
        pipelineConfig.dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

        pipelineConfig.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineConfig.dynamicStateInfo.pDynamicStates = pipelineConfig.dynamicStateEnables.data();
        pipelineConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(pipelineConfig.dynamicStateEnables.size());
        pipelineConfig.dynamicStateInfo.flags = 0;

        pipelineInfo.renderPass = shadowPass;
        pipelineInfo.subpass = 0;
        if (vkCreateGraphicsPipelines( m_VhlDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipelines.offscreen) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline");
        } 

    }
      
    void ShadowRenderSystem::renderShadowMap(FrameInfo& frameInfo, DescriptorSets& bindingDescriptorSets)
    {
        /*
            First render pass: Generate shadow map by rendering the scene from light's POV
        */
        vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.offscreen);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_PipelineLayout,
            0, 1,
            &bindingDescriptorSets.offscreen[frameInfo.frameIndex],
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
    void ShadowRenderSystem::renderGameObjects(FrameInfo& frameInfo, DescriptorSets& bindingDescriptorSets)
    {
        bool debugMode = false;
        /*
            Second pass: Scene rendering with applied shadow map
        */
        if (debugMode)
        {
            vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.debug);
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_PipelineLayout,
                0, 1,
                &bindingDescriptorSets.debug[frameInfo.frameIndex],
                0,
                nullptr);
            vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
        }
        else 
        {
            vkCmdBindPipeline(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VhlShadowMap::filterPCF) ? m_Pipelines.sceneShadowPCF : m_Pipelines.sceneShadow);
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_PipelineLayout,
                0, 1,
                &bindingDescriptorSets.scene[frameInfo.frameIndex],
                0,
                nullptr);

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

    VkPipelineShaderStageCreateInfo ShadowRenderSystem::loadShader(const std::string& filepath, VkShaderStageFlagBits stage)
    {
        auto shaderCode = VhlPipeline::readFile(filepath);
        VkShaderModule shaderModule;

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = shaderCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

        if (vkCreateShaderModule(m_VhlDevice.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create shader module!");
        }

        VkPipelineShaderStageCreateInfo shaderStage{};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = stage;
        shaderStage.module = shaderModule;
        shaderStage.pName = "main";

        m_ShaderModules.push_back(shaderStage.module);

        return shaderStage;
    }
}
