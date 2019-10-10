#include "Window.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN32

#include "GLFW\glfw3.h"

namespace lux
{
	Window::Window()
		: isInitialized(false)
	{

	}

	Window::~Window()
	{

	}

	bool Window::Initialize(uint32_t width, uint32_t height)
	{
		isInitialized = true;

		return true;
	}
}