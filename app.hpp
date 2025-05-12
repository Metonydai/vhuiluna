#pragma once

#include "vhl_window.hpp"
#include "vhl_pipeline.hpp"
#include "vhl_swap_chain.hpp"
#include "vhl_device.hpp"
#include "vhl_model.hpp"

// std
#include <memory>
#include <vector>

namespace vhl 
{
	class HuiApp
	{
	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;

		HuiApp();
		~HuiApp();

		HuiApp(const HuiApp&) = delete;
		HuiApp& operator=(const HuiApp&) = delete;

		void createFractal( std::vector<VhlModel::Vertex>& vertices, int level, glm::vec2 top, glm::vec2 left, glm::vec2 right);

		void run();

	private:
		void loadModels();

		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame(); 
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);

		VhlWindow m_VhlWindow{ WIDTH, HEIGHT, "Hello Huiyu" };
		VhlDevice m_VhlDevice{ m_VhlWindow };
		//VhlSwapChain m_VhlSwapChain{ m_VhlDevice, m_VhlWindow.getExtent() };
		std::unique_ptr<VhlSwapChain> m_VhlSwapChain;
		//VhlPipeline m_VhlPipeline{ m_VhlDevice, "shaders/shader.vert.spv", "shaders/shader.frag.spv", VhlPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT) };
		std::unique_ptr<VhlPipeline> m_VhlPipeline;
		VkPipelineLayout m_PipelineLayout;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::unique_ptr<VhlModel> m_VhlModel;
	};
}