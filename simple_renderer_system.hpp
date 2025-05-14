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
	class SimpleRenderSystem
	{
	public:
		SimpleRenderSystem(VhlDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void loadGameObjects();

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VhlDevice& m_VhlDevice;

		std::unique_ptr<VhlPipeline> m_VhlPipeline;
		VkPipelineLayout m_PipelineLayout;
	};
}
