#include "Engine.h"

namespace lux
{
	Engine::Engine()
		: isInitialized(false), window(), rhi()
	{

	}

	Engine::~Engine()
	{

	}

	bool Engine::Initialize(uint32_t windowWidth, uint32_t windowHeight)
	{
		if (!window.Initialize(windowWidth, windowHeight))
			return false;

		if (!rhi.Initialize(window))
			return false;

		isInitialized = true;

		return true;
	}
} // namespace lux