#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vhl 
{
	class VhlWindow {
	public:
		VhlWindow(int w, int h, std::string name);
		~VhlWindow();

		// Delete copy constructor and copy assignment
		VhlWindow(const VhlWindow&) = delete;
		VhlWindow& operator=(const VhlWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(m_Window); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; }
		bool wasWindowResized() const { return m_FramebufferResized; }
		void resetWindowResizedFlag() { m_FramebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* pSurface);

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		void initWindow();

		int m_Width;
		int m_Height;
		bool m_FramebufferResized = false;

		std::string m_WindowName;
		GLFWwindow* m_Window;
	};
}