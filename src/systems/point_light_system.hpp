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
	class PointLightSystem
	{
	public:
		PointLightSystem(VhlDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VhlDevice& m_VhlDevice;

		std::unique_ptr<VhlPipeline> m_VhlPipeline;
		VkPipelineLayout m_PipelineLayout;
	};
}
