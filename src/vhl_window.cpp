#include "vhl_window.hpp"

#include <stdexcept>

namespace vhl 
{
	VhlWindow::VhlWindow(int w, int h, std::string name) : m_Width(w), m_Height(h), m_WindowName(name)
	{
		initWindow();
	}

	VhlWindow::~VhlWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void VhlWindow::initWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
	}

	void VhlWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* pSurface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, pSurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
	}
	
	void VhlWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) 
	{
		auto vhlWindow = reinterpret_cast<VhlWindow*>(glfwGetWindowUserPointer(window));
		vhlWindow->m_FramebufferResized = true;
		vhlWindow->m_Width = width;
		vhlWindow->m_Height = height;
	}

}