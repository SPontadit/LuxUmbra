#include "Window.h"

namespace lux
{
	Window::Window() noexcept
		: isInitialized(false), window(nullptr)
	{

	}

	Window::~Window() noexcept
	{
		if (isInitialized)
		{
			if (window != nullptr)
				glfwDestroyWindow(window);

			glfwTerminate();
		}
	}

	bool Window::Initialize(uint32_t width, uint32_t height) noexcept
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Luxumbra_demo", nullptr, nullptr);
		if (!window)
		{
			glfwTerminate();
			return false;
		}

		isInitialized = true;

		return true;
	}

	HWND Window::GetHandle() const noexcept
	{
		return glfwGetWin32Window(window);
	}

	void Window::PollEvents() const noexcept
	{
		glfwPollEvents();
	}

	bool Window::ShouldClose() const noexcept
	{
		return glfwWindowShouldClose(window);
	}

} // namespace lux