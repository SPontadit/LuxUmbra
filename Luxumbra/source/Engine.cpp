#include "Engine.h"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scene(), resourceManager(rhi)
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

		resourceManager.Initialize();

		scene.Initialize(window, resourceManager);

		isInitialized = true;

		return true;
	}

	void Engine::Run() noexcept
	{
		while (!window.ShouldClose())
		{
			rhi.RenderForward(scene.GetCurrentCamera(), scene.GetMeshNodes());

			window.PollEvents();
		}
	}

	scene::Scene& Engine::GetScene() noexcept
	{
		return scene;
	}

} // namespace lux