#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <stdint.h>

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

	private:
		bool isInitialized;
	};

} // namespace lux

#endif // WINDOW_H_INCLUDED