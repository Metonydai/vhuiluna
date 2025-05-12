#pragma once

#include "vhl_device.hpp"
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
		SimpleRenderSystem(VhlDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<VhlGameObject>& gameObjects);

	private:
		void loadGameObjects();

		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		VhlDevice& m_VhlDevice;

		std::unique_ptr<VhlPipeline> m_VhlPipeline;
		VkPipelineLayout m_PipelineLayout;
	};
}
