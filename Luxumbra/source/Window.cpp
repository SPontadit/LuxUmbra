#include "Window.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

namespace lux
{
	Window* luxWindow = nullptr;

	void WindowFocusCallback(GLFWwindow* window, int hasFocus);
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept;

	Window::Window() noexcept
		: isInitialized(false), width(0), height(0), window(nullptr), hasFocus(true),
		actionsStatus{ false }, mouseXDelta(0.f), mouseYDelta(0.f), deltaTime(0.f)
	{
		luxWindow = this;
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

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

		window = glfwCreateWindow(videoMode->width, videoMode->height, "Luxumbra_demo", monitor, nullptr);
		if (!window)
		{
			glfwTerminate();
			return false;
		}

		this->width = width;
		this->height = height;

		glfwSetWindowAttrib(window, GLFW_RESIZABLE, 0);

		glfwSetWindowFocusCallback(window, WindowFocusCallback);

		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);

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

	void Window::PollEvents() noexcept
	{
		glfwPollEvents();

		static double oldX = 0., oldY = 0.;
		static double oldTime = 0.;

		// Mouse movement

		double x, y;
		glfwGetCursorPos(window, &x, &y);

		mouseXDelta = TO_FLOAT(x - oldX);
		mouseYDelta = TO_FLOAT(y - oldY);

		oldX = x;
		oldY = y;

		// Delta time

		double time = glfwGetTime();
		deltaTime = TO_FLOAT(time - oldTime);
		oldTime = time;
	}

	bool Window::ShouldClose() const noexcept
	{
		return glfwWindowShouldClose(window);
	}

	float Window::GetAspect() const noexcept
	{
		return TO_FLOAT(width) / TO_FLOAT(height);
	}

	bool Window::GetHasFocus() const noexcept
	{
		return hasFocus;
	}

	void Window::SetHasFocus(bool newHasFocus) noexcept
	{
		hasFocus = newHasFocus;
	}

	bool Window::GetActionsStatus(Action action) const noexcept
	{
		return actionsStatus[TO_SIZE_T(action)];
	}
	
	void Window::SetActionStatus(Action action, bool status) noexcept
	{
		actionsStatus[TO_SIZE_T(action)] = status;
	}

	void Window::GetMouseDelta(float& x, float& y) const noexcept
	{
		x = mouseXDelta;
		y = mouseYDelta;
	}

	float Window::GetDeltaTime() const noexcept
	{
		return deltaTime;
	}

	void WindowFocusCallback(GLFWwindow* window, int hasFocus)
	{
		luxWindow->SetHasFocus(hasFocus != GLFW_FALSE);
	}

	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept
	{
		switch (key)
		{
		case GLFW_KEY_W:
		case GLFW_KEY_UP:
			luxWindow->SetActionStatus(Action::FORWARD, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_S:
		case GLFW_KEY_DOWN:
			luxWindow->SetActionStatus(Action::BACKWARD, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_D:
		case GLFW_KEY_RIGHT:
			luxWindow->SetActionStatus(Action::RIGHT, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_A:
		case GLFW_KEY_LEFT:
			luxWindow->SetActionStatus(Action::LEFT, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_SPACE:
		case GLFW_KEY_E:
			luxWindow->SetActionStatus(Action::UP, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_LEFT_ALT:
		case GLFW_KEY_C:
			luxWindow->SetActionStatus(Action::DOWN, action != GLFW_RELEASE);
			break;

		case GLFW_KEY_I:
		{
			static int lastAction = GLFW_RELEASE;

			luxWindow->SetActionStatus(Action::TOGGLE_UI, lastAction != GLFW_PRESS && action == GLFW_PRESS);

			lastAction = action;

			break;
		}

		case GLFW_KEY_ESCAPE:
			luxWindow->SetActionStatus(Action::EXIT, action != GLFW_RELEASE);
			break;
		}
	}

	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_2:
			luxWindow->SetActionStatus(Action::FREE_LOOK, action != GLFW_RELEASE);
			break;
		}
	}

} // namespace lux