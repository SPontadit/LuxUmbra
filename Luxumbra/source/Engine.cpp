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

		scene.Initialize(window, rhi, resourceManager);

		isInitialized = true;


		return true;
	}

	void Engine::Run() noexcept
	{
		while (!window.ShouldClose())
		{
			DrawImgui();

			rhi.Render(scene.GetCurrentCamera(), scene.GetMeshNodes(), scene.GetLightNodes());

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
		//ImGuiStyle& style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Luxumbra Engine");

		if (ImGui::Button("Reload Shader"))
		{
			rhi.RebuildForwardGraphicsPipeline();
		}

		ImGui::NewLine();

		if (ImGui::BeginTabBar("##TabBar"))
		{
			if (ImGui::BeginTabItem("Post Process"))
			{
				lux::rhi::PostProcessParameters& postProcess = rhi.forward.postProcessParameters;
				
				if (ImGui::CollapsingHeader("ToneMapping:", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Spacing();

					int toneMapping = postProcess.toneMapping;
					int newToneMapping = toneMapping;
					ImGui::Combo("Tone Mapping", &newToneMapping, "ACES Film\0Uncharted 2\0Reinhard");

					if (newToneMapping != toneMapping)
						postProcess.toneMapping = newToneMapping;

					ImGui::Spacing();

					// Exposure
					float exposure = postProcess.exposure;
					float newExposure = exposure;
					ImGui::SliderFloat("Exposure", &newExposure, 0.0f, 10.0f, "%.2f");

					if (newExposure != exposure)
						postProcess.exposure = newExposure;
				
					ImGui::NewLine();
				}

				if (ImGui::CollapsingHeader("FXAA:", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Spacing();

					int FXAAQuality;
					if (postProcess.FXAARelativeThreshold == 0.250f) FXAAQuality = 0;
					else if (postProcess.FXAARelativeThreshold == 0.166f) FXAAQuality = 1;
					else if (postProcess.FXAARelativeThreshold == 0.125f) FXAAQuality = 2;

					int newFXAAQuality = FXAAQuality;

					const char* qualityName[3] = { "Low", "Medium", "High" };
					ImGui::SliderInt("Quality", &newFXAAQuality, 0, 2, qualityName[newFXAAQuality]);

					if (newFXAAQuality != FXAAQuality)
					{
						// Low
						if (newFXAAQuality == 0)
						{
							postProcess.FXAAContrastThreshold = 0.0833;
							postProcess.FXAARelativeThreshold = 0.250f;

						}
						// Medium
						else if (newFXAAQuality == 1)
						{
							postProcess.FXAAContrastThreshold = 0.0625;
							postProcess.FXAARelativeThreshold = 0.166f;

						}
						// High
						else
						{
							postProcess.FXAAContrastThreshold = 0.0312f;
							postProcess.FXAARelativeThreshold = 0.125f;
						}
					}

					ImGui::Spacing();

					bool debugFXAA = postProcess.FXAADebug;
					bool newDebugFXAA = debugFXAA;
					ImGui::Checkbox("Show Split View", &newDebugFXAA);

					if (newDebugFXAA != debugFXAA)
						postProcess.FXAADebug = newDebugFXAA;

					if (newDebugFXAA)
					{
						ImGui::Spacing();

						// FXAA Percent
						float FXAAPercent = postProcess.FXAAPercent;
						float newFXAAPercent = FXAAPercent;
						ImGui::SliderFloat("Percent", &newFXAAPercent, 0.0f, 1.0f, "%.2f");
				
						if (newFXAAPercent != FXAAPercent)
							postProcess.FXAAPercent = newFXAAPercent;
					}

					ImGui::NewLine();
				}


				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Scene"))
			{
				scene::CameraNode* camera = scene.GetCurrentCamera();
				DisplayCameraNode(camera);
				ImGui::NewLine();

				DisplayMeshNodes(scene.GetMeshNodes());
				ImGui::NewLine();
				DisplayLightNodes(scene.GetLightNodes());
				ImGui::NewLine();

				DisplayMaterials();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
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

					ImGui::TreePop();
				}
			}
		
			ImGui::TreePop();
		}
	}

	void Engine::DisplayMaterials() noexcept
	{
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

						if (matParameters.metallic == false)
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
