#include "Window.h"

namespace lux
{
	Window::Window()
		: isInitialized(false), window(nullptr, glfwDestroyWindow)
	{

	}

	Window::~Window()
	{

	}

	bool Window::Initialize(uint32_t width, uint32_t height)
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window.reset(glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Luxumbra_demo", nullptr, nullptr));
		if (!window)
		{
			glfwTerminate();
			return false;
		}

		isInitialized = true;

		return true;
	}

	HWND Window::GetHandle() const
	{
		return glfwGetWin32Window(window.get());
	}
} // neamspace lux