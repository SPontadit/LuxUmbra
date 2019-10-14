#include "Engine.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

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

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_InitForVulkan(window.GetGLFWWindow(), true);

		rhi.InitImgui();

		resourceManager.Initialize();

		scene.Initialize(window, resourceManager);

		isInitialized = true;


		return true;
	}

	void Engine::Run() noexcept
	{
		while (!window.ShouldClose())
		{
			DrawImgui();

			rhi.RenderForward(scene.GetCurrentCamera(), scene.GetMeshNodes());

			window.PollEvents();
		}
	}

	scene::Scene& Engine::GetScene() noexcept
	{
		return scene;
	}

	void Engine::DrawImgui() noexcept
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Luxumbra Engine");
		
		ImGui::End();

		ImGui::Render();
	}

} // namespace lux
