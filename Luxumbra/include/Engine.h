#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "Luxumbra.h"

#include "Window.h"
#include "rhi\RHI.h"
#include "scene\Scene.h"
#include "resource/ResourceManager.h"


namespace lux
{

	class Engine
	{
	public:
		Engine() noexcept;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;

		~Engine() noexcept;

		const Engine& operator=(const Engine&) = delete;
		const Engine& operator=(Engine&&) = delete;

		bool Initialize(uint32_t windowWidth, uint32_t windowHeight) noexcept;
		void Run() noexcept;

		scene::Scene& GetScene() noexcept;

	private:
		bool isInitialized;

		Window window;
		rhi::RHI rhi;
		scene::Scene scene;
		resource::ResourceManager resourceManager;
	};

} // namespace lux

#endif // ENGINE_H_INCLUDED