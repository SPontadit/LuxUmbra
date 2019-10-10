#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include "Luxumbra.h"


#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN32

#include "GLFW\glfw3.h"
#include "GLFW\glfw3native.h"

namespace lux
{

	class Window
	{
	public:
		Window();
		Window(const Window&) = delete;
		Window(Window&&) = delete;

		~Window();

		const Window& operator=(const Window&) = delete;
		const Window& operator=(Window&&) = delete;

		bool Initialize(uint32_t width, uint32_t height);

		HWND GetHandle() const;

	private:
		bool isInitialized;

		std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window;
	};

} // namespace lux

#endif // WINDOW_H_INCLUDED