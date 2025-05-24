#pragma once

#include "vhl_camera.hpp"
#include "vhl_device.hpp"
#include "vhl_frame_info.hpp"
#include "vhl_game_object.hpp"
#include "vhl_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace vhl 
{
	class ShadowRenderSystem
	{
	public:
		ShadowRenderSystem(VhlDevice& device, VkRenderPass shadowPass, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~ShadowRenderSystem();

		ShadowRenderSystem(const ShadowRenderSystem&) = delete;
		ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUBO& ubo);
		void renderShadowMap(FrameInfo& frameInfo, DescriptorSets& bindingDescriptorSets);
		void renderGameObjects(FrameInfo& frameInfo, DescriptorSets& bindingDescriptorSets);

	private:
		VkPipelineShaderStageCreateInfo loadShader(const std::string& filepath, VkShaderStageFlagBits stage);
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipelines(VkRenderPass renderPass0, VkRenderPass renderPass1);

		VhlDevice& m_VhlDevice;

		struct {
			VkPipeline offscreen;
			VkPipeline sceneShadow;
			// Pipeline with percentage close filtering (PCF) of the shadow map 
			VkPipeline sceneShadowPCF;
			VkPipeline debug;
		} m_Pipelines;

		std::vector<VkShaderModule> m_ShaderModules;

		VkPipelineLayout m_PipelineLayout;
	};
}
