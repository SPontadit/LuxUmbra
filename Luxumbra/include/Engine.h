#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "Luxumbra.h"

#include "Window.h"
#include "rhi\RHI.h"
#include "scene\Scene.h"

namespace lux
{

	class Engine
	{
	public:
		Engine() noexcept;
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;

		~Engine() noexcept = default;

		const Engine& operator=(const Engine&) = delete;
		const Engine& operator=(Engine&&) = delete;

		bool Initialize(uint32_t windowWidth, uint32_t windowHeight) noexcept;
		void Run() noexcept;

	private:
		bool isInitialized;

		Window window;
		rhi::RHI rhi;
		scene::Scene scene;
	};

} // namespace lux

#endif // ENGINE_H_INCLUDED