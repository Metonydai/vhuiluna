#pragma once

#include "vhl_device.hpp"
#include "vhl_game_object.hpp"
#include "vhl_renderer.hpp"
#include "vhl_window.hpp"

// std
#include <memory>
#include <vector>

namespace vhl 
{
	class HuiApp
	{
	public:
		static constexpr int WIDTH = 1080;
		static constexpr int HEIGHT = 1080;

		HuiApp();
		~HuiApp();

		HuiApp(const HuiApp&) = delete;
		HuiApp& operator=(const HuiApp&) = delete;

		void createFractal( std::vector<VhlModel::Vertex>& vertices, int level, glm::vec2 top, glm::vec2 left, glm::vec2 right);

		void run();

	private:
		void loadGameObjects();

		VhlWindow m_VhlWindow{ WIDTH, HEIGHT, "Hello Huiyu" };
		VhlDevice m_VhlDevice{ m_VhlWindow };
		VhlRenderer m_VhlRenderer{ m_VhlWindow, m_VhlDevice };

		std::vector<VhlGameObject> m_GameObjects;
	};
}