#include "Engine.h"

#include <set>

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

			rhi.Render(scene.GetCurrentCamera(), scene.GetShadowCastingDirectional(), scene.GetMeshNodes(), scene.GetLightNodes());

			window.PollEvents();
		}
	}

	scene::Scene& Engine::GetScene() noexcept
	{
		return scene;
	}

	resource::ResourceManager& Engine::GetResourceManager() noexcept
	{
		return resourceManager;
	}

	void Engine::DrawImgui() noexcept
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Luxumbra Engine");
		
		if (ImGui::Button("Reload Shader"))
		{
			rhi.RebuildForwardGraphicsPipeline();
		}

		ImGui::NewLine();

		scene::CameraNode* camera = scene.GetCurrentCamera();
		DisplayCameraNode(camera);
		ImGui::NewLine();

		DisplayMeshNodes(scene.GetMeshNodes());
		ImGui::NewLine();
		DisplayLightNodes(scene.GetLightNodes());
		ImGui::NewLine();


		if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::set<std::string> materialNames;
			const std::vector<scene::MeshNode*>& meshNodes = scene.GetMeshNodes();
			for (size_t i = 0; i < meshNodes.size(); i++)
			{
				resource::Material* currentMaterial = &meshNodes[i]->GetMaterial();
				std::set<std::string>::iterator mat = materialNames.find(currentMaterial->name);

				if (mat == materialNames.end())
				{
					materialNames.insert(currentMaterial->name);

					if (ImGui::TreeNode(currentMaterial->name.c_str()))
					{
						resource::MaterialParameters& matParameters = currentMaterial->parameter;

						ImGui::ColorEdit3("Base Color", glm::value_ptr(matParameters.baseColor));
						bool metallic = (bool)matParameters.metallic;
						ImGui::Checkbox("Metallic", &metallic);
						matParameters.metallic = metallic;

						if (matParameters.metallic == 0.f)
						{
							ImGui::SliderFloat("Reflectance", &matParameters.reflectance, 0.0f, 1.0f, "%.3f");
						}

						ImGui::SliderFloat("Roughness", &matParameters.perceptualRoughness, 0.0f, 1.0f, "%.3f");

						ImGui::TreePop();
					}
				}
			}
			ImGui::TreePop();
		}

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
				std::string displayName("Mesh " + std::to_string(i) + " | " + meshes[i]->GetMaterial().name);
				if (ImGui::TreeNode(displayName.c_str()))
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
				{
					DisplayNode(currentLight);

					glm::vec3 color = currentLight->GetColor();
					ImGui::ColorEdit3("Color", glm::value_ptr(color));
					
					if (color != currentLight->GetColor())
					{
						currentLight->SetColor(color);
					}

					int lightType = static_cast<int>((currentLight->GetType()));
					int newLightType = lightType;
					ImGui::Combo("Light Type", &newLightType, "Directional\0Point");
					
					if (newLightType != lightType)
					{
						currentLight->SetType(static_cast<scene::LightType>(newLightType));
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

		ImGui::DragFloat3("Pos", glm::value_ptr(position), 0.01f);

		if (position != node->GetLocalPosition())
		{
			node->SetLocalPosition(position);
		}

		// Rotation
		glm::vec3 rotation = glm::degrees(node->GetLocalRotation());

		ImGui::DragFloat3("Rot", glm::value_ptr(rotation));

		if (rotation != node->GetLocalRotation())
		{
			node->SetLocalRotation(glm::radians(rotation));
		}

		// Rotation
		glm::vec3 scale = node->GetLocalScale();

		ImGui::DragFloat3("Scale", glm::value_ptr(scale));

		if (scale != node->GetLocalScale())
		{
			node->SetLocalScale(scale);
		}
	}


} // namespace lux
