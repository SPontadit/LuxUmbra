#include "Engine.h"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scene()
	{

	}

	bool Engine::Initialize(uint32_t windowWidth, uint32_t windowHeight) noexcept
	{
		if (!window.Initialize(windowWidth, windowHeight))
			return false;

		if (!rhi.Initialize(window))
			return false;

		if (!scene.Initialize(window))
			return false;

		isInitialized = true;

		return true;
	}

	void Engine::Run() noexcept
	{
		while (!window.ShouldClose())
		{
			rhi.RenderForward();

			window.PollEvents();
		}
	}

} // namespace lux