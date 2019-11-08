#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#include "Luxumbra.h"

#include "Window.h"
#include "rhi\RHI.h"
#include "scene\Scene.h"
#include "resource/ResourceManager.h"


namespace lux
{
	enum class SCENE : int32_t
	{
		SPHERE_SCENE = 0,
		POST_PROCESS_SCENE,
		PBR_MODELS_SCENE,
		PBR_MATERIALS_SCENE,
		TRANSPARENT_SCENE,
		DIRECTIONAL_SHADOW_SCENE,
		SCENE_COUNT
	};

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

		scene::Scene& GetScene(SCENE scene) noexcept;
		resource::ResourceManager& GetResourceManager() noexcept;

	private:
		void Update(scene::Scene& scene) noexcept;
		void DrawImgui(scene::Scene& scene) noexcept;
		void DisplayCameraNode(scene::CameraNode* node) noexcept;
		void DisplayMeshNodes(const std::vector<scene::MeshNode*>& meshes) noexcept;
		void DisplayLightNodes(const std::vector<scene::LightNode*>& lights) noexcept;
		void DisplayMaterials(const std::vector<scene::MeshNode*>& meshes) noexcept;
		void DisplayNode(scene::Node* node) noexcept;

		bool isInitialized;

		Window window;
		rhi::RHI rhi;
		std::array<scene::Scene, TO_SIZE_T(SCENE::SCENE_COUNT)> scenes;
		resource::ResourceManager resourceManager;

		int32_t currentScene;

		double currentTime;
		double previousTime;

		bool drawGUI;
	};

} // namespace lux

#endif // ENGINE_H_INCLUDED