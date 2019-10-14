#include "Engine.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

#include "glm\gtc\type_ptr.hpp"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scene(), resourceManager(rhi), imguiID(0)
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
		

		scene::CameraNode* camera = scene.GetCurrentCamera();
		DisplayCameraNode(camera);
		ImGui::NewLine();

		const std::vector<scene::MeshNode*> nodes = scene.GetMeshNodes();
		for (size_t i = 0; i < nodes.size(); i++)
		{
			DisplayMeshNode(nodes[i]);
		}

		ImGui::End();

		ImGui::Render();

		imguiID = 0;
	}

	void Engine::DisplayCameraNode(scene::CameraNode* node) noexcept
	{
		ImGui::PushID(imguiID);
		++imguiID;

		DisplayNode(node, "Camera");

		// TODO: Expose Camera relevant stuff

		ImGui::PopID();
	}

	void Engine::DisplayMeshNode(scene::MeshNode* node) noexcept
	{
		ImGui::PushID(imguiID);
		++imguiID;

		DisplayNode(node, "Mesh");

		ImGui::PopID();
	}

	void Engine::DisplayNode(scene::Node* node, const std::string& name) noexcept
	{
		ImGui::PushID(imguiID);
		++imguiID;

		std::string posID = "Pos";
		std::string rotID = "Rot";

		ImGui::Text(name.c_str());

		//Position
		glm::vec3 position = node->GetLocalPosition();

		ImGui::DragFloat3(posID.c_str(), glm::value_ptr(position));

		if (position != node->GetLocalPosition())
		{
			node->SetLocalPosition(position);
		}

		////// Rotation
		//glm::quat rotation = nodes[i]->GetLocalRotation();
		//glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(rotation));

		//ImGui::DragFloat3(rotID.c_str(), glm::value_ptr(eulerRotation));

		//glm::mat4 rot = glm::identity<glm::mat4>();
		//rot = glm::rotate(rot, glm::radians(eulerRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//rot = glm::rotate(rot, glm::radians(eulerRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		//rot = glm::rotate(rot, glm::radians(eulerRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		//glm::quat newRotation = glm::toQuat(rot);

		//if (newRotation != rotation)
		//{
		//	nodes[i]->SetLocalRotation(newRotation);
		//}

		ImGui::PopID();
	}


} // namespace lux
