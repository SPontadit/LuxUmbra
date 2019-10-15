#include "Engine.h"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_glfw.h"
#include "imgui\imgui_impl_vulkan.h"

#include "glm\gtc\type_ptr.hpp"

namespace lux
{

	Engine::Engine() noexcept
		: isInitialized(false), window(), rhi(), scene(), resourceManager(rhi)
	{

	}

	Engine::~Engine()
	{
		rhi.WaitIdle();
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
			DrawImgui();

			rhi.RenderForward(scene.GetCurrentCamera(), scene.GetMeshNodes(), scene.GetLightNodes());

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

		DisplayMeshNodes(scene.GetMeshNodes());
		ImGui::NewLine();
		DisplayLightNodes(scene.GetLightNodes());

		ImGui::End();

		ImGui::Render();
	}

	void Engine::DisplayCameraNode(scene::CameraNode* node) noexcept
	{
		if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayNode(node);

			// TODO: Expose Camera relevant stuff
			
			ImGui::TreePop();
		}
	}

	void Engine::DisplayMeshNodes(const std::vector<scene::MeshNode*>& meshes) noexcept
	{
		size_t meshCount = meshes.size();
		scene::MeshNode* currentMesh;

		if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < meshCount; i++)
			{
				if (ImGui::TreeNodeEx(("Mesh " + std::to_string(i)).c_str()))
				//if (ImGui::TreeNode((void*)(intptr_t)i, "Mesh %d", i))
				{
					currentMesh = meshes[i];
					DisplayNode(currentMesh);
				
					ImGui::TreePop();
				}
			}
			
			ImGui::TreePop();
		}
	}

	void Engine::DisplayLightNodes(const std::vector<scene::LightNode*>& lights) noexcept
	{
		size_t lightCount = lights.size();
		scene::LightNode* currentLight;

		if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (size_t i = 0; i < lightCount; i++)
			{
				currentLight = lights[i];

				if (ImGui::TreeNode(("Light " + std::to_string(i)).c_str()))
					//if (ImGui::TreeNode((void*)(intptr_t)i, "Light %d", i))
				{
					DisplayNode(currentLight);

					glm::vec3 color = currentLight->GetColor();
					ImGui::ColorEdit3("Color", glm::value_ptr(color));
					
					if (color != currentLight->GetColor())
					{
						currentLight->SetColor(color);
					}
					
					ImGui::TreePop();
				}
			}
		
			ImGui::TreePop();
		}
	}

	void Engine::DisplayNode(scene::Node* node) noexcept
	{
		//Position
		glm::vec3 position = node->GetLocalPosition();

		ImGui::DragFloat3("Pos", glm::value_ptr(position));

		if (position != node->GetLocalPosition())
		{
			node->SetLocalPosition(position);
		}

		// Rotation
		glm::quat rotation = node->GetLocalRotation();
		glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(rotation));

		ImGui::DragFloat3("Rot", glm::value_ptr(eulerRotation));

		glm::quat newRotation = glm::quat(glm::radians(eulerRotation));

		if (newRotation != rotation)
		{
			node->SetLocalRotation(newRotation);
		}
	}


} // namespace lux
