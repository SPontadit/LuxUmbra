#include "Window.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

namespace lux
{
	Window::Window() noexcept
		: isInitialized(false), width(0), height(0), window(nullptr)
	{

	}

	Window::~Window() noexcept
	{
		if (isInitialized)
		{
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();

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

		this->width = width;
		this->height = height;

		glfwSetWindowAttrib(window, GLFW_RESIZABLE, 0);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_InitForVulkan(window, true);

		isInitialized = true;

		return true;
	}

	HWND Window::GetHandle() const noexcept
	{
		return glfwGetWin32Window(window);
	}

	GLFWwindow* Window::GetGLFWWindow() const noexcept
	{
		return window;
	}

	void Window::PollEvents() const noexcept
	{
		glfwPollEvents();
	}

	bool Window::ShouldClose() const noexcept
	{
		return glfwWindowShouldClose(window);
	}

	float Window::GetAspect() const noexcept
	{
		return TO_FLOAT(width) / TO_FLOAT(height);
	}

} // namespace lux