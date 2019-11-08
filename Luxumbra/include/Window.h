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

	enum class Action : int32_t
	{
		FREE_LOOK = 0,
		FORWARD,
		BACKWARD,
		RIGHT,
		LEFT,
		UP,
		DOWN,
		TOGGLE_UI,

		ACTION_MAX_ENUM
	};

	class Window
	{
	public:
		Window() noexcept;
		Window(const Window&) = delete;
		Window(Window&&) = delete;

		~Window() noexcept;

		const Window& operator=(const Window&) = delete;
		const Window& operator=(Window&&) = delete;

		bool Initialize(uint32_t width, uint32_t height) noexcept;
		HWND GetHandle() const noexcept;
		GLFWwindow* GetGLFWWindow() const noexcept;

		void PollEvents() noexcept;
		bool ShouldClose() const noexcept;

		float GetAspect() const noexcept;

		bool GetHasFocus() const noexcept;
		void SetHasFocus(bool newHasFocus) noexcept;

		bool GetActionsStatus(Action action) const noexcept;
		void SetActionStatus(Action action, bool status) noexcept;
		void GetMouseDelta(float& x, float& y) const noexcept;

		float GetDeltaTime() const noexcept;

	private:
		bool isInitialized;

		uint32_t width, height;
		GLFWwindow* window;

		bool hasFocus;

		bool actionsStatus[TO_SIZE_T(Action::ACTION_MAX_ENUM)];
		float mouseXDelta, mouseYDelta;

		float deltaTime;
	};

} // namespace lux

#endif // WINDOW_H_INCLUDED