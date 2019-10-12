#include "Engine.h"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi()
	{

	}

	Engine::~Engine()
	{

	}

	bool Engine::Initialize(uint32_t windowWidth, uint32_t windowHeight) noexcept
	{
		if (!window.Initialize(windowWidth, windowHeight))
			return false;

		if (!rhi.Initialize(window))
			return false;

		isInitialized = true;

		return true;
	}

	bool Engine::Run() noexcept
	{
		while (true)
		{
			rhi.RenderForward();
		}
	}

} // namespace lux